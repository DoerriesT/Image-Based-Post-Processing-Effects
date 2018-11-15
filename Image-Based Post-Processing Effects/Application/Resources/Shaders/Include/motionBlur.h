
#ifndef TILE_SIZE
#define TILE_SIZE (40)
#endif // TILE_SIZE

#ifndef MAX_SAMPLES
#define MAX_SAMPLES (25)
#endif // MAX_SAMPLES


float hash12(vec2 p)
{
	const float HASHSCALE1 = 443.8975;
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float interleavedGradientNoise(vec2 v)
{
	vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return fract(magic.z * dot(v, magic.xy));
}

float softDepthCompare(float a, float b)
{
	const float SOFT_Z_EXTENT = 0.1;
	return clamp(1.0 - (a - b) / SOFT_Z_EXTENT, 0.0, 1.0);
}

float cone(float dist, float velocityMag)
{
	return velocityMag > 0.0 ? clamp(1.0 - dist / velocityMag, 0.0, 1.0) : 0.0;
}

float cylinder(float dist, float velocityMag)
{
	return 1.0 - smoothstep(0.95 * velocityMag, 1.05 * velocityMag, dist);
}

float linearDepth(float depth, float near, float far)
{
    float z_n = 2.0 * depth - 1.0;
    return 2.0 * near * far / (far + near - z_n * (far - near));
}

// Jitter function for tile lookup
vec2 jitterTile(vec2 uv, vec2 texturesize, vec2 tileTexelSize)
{
	float val = interleavedGradientNoise((uv + vec2(2.0, 0.0)) * texturesize) * PI * 2.0;
	return vec2(sin(val), cos(val)) * tileTexelSize * 0.25;
}

vec3 simpleMotionBlur(vec3 color, vec2 texCoord, ivec2 fragCoord, sampler2D colorTexture, sampler2D velocityTexture)
{
	vec2 texSize = textureSize(colorTexture, 0);
	
	vec2 velocity = texture(velocityTexture, texCoord).rg * texSize;

	float speed = length(velocity);
	float sampleCount = min(floor(speed), MAX_SAMPLES);
	
	if (sampleCount < 1.0)
	{
		return color;
	}

	color = vec3(0.0);
	for (float i = 0; i < sampleCount; ++i) 
	{
		float t = mix(-1.0, 1.0, i / (sampleCount -1.0));
		ivec2 offset = ivec2(velocity * t);
		color += texelFetch(colorTexture, fragCoord + offset, 0).rgb;
	}
	
	color /= sampleCount;
	
	return color;
}

vec3 singleDirectionMotionBlur(vec3 color, vec2 texCoord, ivec2 fragCoord, sampler2D colorTexture, sampler2D velocityTexture, sampler2D depthTexture, sampler2D tileTexture, float near, float far)
{
	vec2 texSize = textureSize(colorTexture, 0);
	
	vec2 vN = texture(tileTexture, texCoord).rg * texSize;
	
	if (dot(vN, vN) < 0.25)
	{
		return color;
	}

	vec2 vC = texelFetch(velocityTexture, fragCoord, 0).rg * texSize;

	float depthC = -linearDepth(texelFetch(depthTexture, fragCoord, 0).x, near, far);

	float vCLength = max(length(vC), 0.5);
	float vNLength = length(vN);

	float weight = 1.0 / vCLength;
	vec3 sum = color * weight;

	float j = 0.0;

	const int N = MAX_SAMPLES;

	for (int i = 0; i < N; ++i)
	{
		if (i == N / 2)
		{
			continue;
		}

		float t = mix(-1.0, 1.0, (i + j + 1.0) / (N + 1.0));
		ivec2 S = fragCoord + ivec2(vN * t + 0.5);

		float depthS = -linearDepth(texelFetch(depthTexture, S, 0).x, near, far);

		float f = softDepthCompare(depthC, depthS);
		float b = softDepthCompare(depthS, depthC);

		vec2 vS = texelFetch(velocityTexture, S, 0).rg * texSize;

		float vSLength = max(length(vS), 0.5);

		float dist = abs(t) * vNLength;

		float alpha = f * cone(dist, vSLength)
					+ b * cone(dist, vCLength)
					+ cylinder(dist, vSLength)
					* cylinder(dist, vCLength) * 2.0;

		weight += alpha;
		sum += alpha * texelFetch(colorTexture, S, 0).rgb;
	}
	
	color = sum / weight;
	
	return color;
}

vec3 multiDirectionMotionBlur(vec3 color, vec2 texCoord, ivec2 fragCoord, sampler2D colorTexture, sampler2D velocityTexture, sampler2D depthTexture, sampler2D tileTexture, float near, float far)
{
	vec2 texSize = textureSize(colorTexture, 0);
	
	float j = interleavedGradientNoise(texCoord * texSize);//(hash12(vTexCoord) - 0.5) * 2.0;
	vec2 vmax = texture(tileTexture, texCoord + jitterTile(texCoord, texSize, 1.0 / textureSize(tileTexture, 0).xy)).rg * texSize;
	float vmaxLength = length(vmax);
	
	if (vmaxLength < 0.5)
	{
		return color;
	}

	vec2 wN = vmax / vmaxLength;
	vec2 vC = texelFetch(velocityTexture, fragCoord, 0).rg * texSize;
	vec2 wP = vec2(-wN.y, wN.x);
	wP = (dot(wP, vC) < 0.0) ? -wP : wP;

	float vCLength = max(length(vC), 0.5);
	const float velocityThreshold = 1.5;
	vec2 wC = normalize(mix(wP, vC / vCLength, (vCLength - 0.5) / velocityThreshold));

	const float N = MAX_SAMPLES;
	
	float totalWeight = N / (TILE_SIZE * vCLength);
	vec3 result = color * totalWeight;
	
	float depthC = -linearDepth(texelFetch(depthTexture, fragCoord, 0).x, near, far);

	vec2 dN[2] = { wN, vC / vCLength };

	for (int i = 0; i < N; ++i)
	{
		float t = mix(-1.0, 1.0, (i + j * 0.95 + 1.0) / (N + 1.0));
	
		vec2 d = bool(i & 1) ? vC : vmax;
		float T = abs(t) * vmaxLength;
		ivec2 S = fragCoord + ivec2(t * d);
	
		float depthS = -linearDepth(texelFetch(depthTexture, S, 0).x, near, far);
	
		float f = softDepthCompare(depthC, depthS);
		float b = softDepthCompare(depthS, depthC);
	
		vec2 vS = texelFetch(velocityTexture, S, 0).rg * texSize;

		float vSLength = max(length(vS), 0.5);

		int index = i & 1;
		float wA = abs(dot(wC, dN[index]));
		float wB = abs(dot(vS / vSLength, dN[index]));
		
		float weight = 0.0;

		weight += f * cone(T, vSLength) * wB;
		weight += b * cone(T, vCLength) * wA;
		// originally max(wA, wB), but using min reduces artifacts in extreme conditions
		weight += cylinder(T, min(vSLength, vCLength)) * max(wA, wB) * 2.0;
	
		totalWeight += weight;
		result += texelFetch(colorTexture, S, 0).rgb * weight;
	}
	
	color = result / totalWeight;
	
	return color;
}