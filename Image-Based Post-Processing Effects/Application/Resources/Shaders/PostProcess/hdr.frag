#version 330 core

out vec4 oFragColor;
in vec2 vTexCoord;

uniform sampler2D uScreenTexture; // full resolution source texture
uniform sampler2D uBloomTexture;
uniform sampler2D uVelocityTexture;
uniform sampler2D uVelocityNeighborMaxTexture;
uniform sampler2D uDepthTexture;
uniform sampler2D uLensFlareTex; // input from the blur stage
uniform sampler2D uLensDirtTex; // full resolution dirt texture
uniform sampler2D uLensStarTex; // diffraction starburst texture
uniform sampler2D uDofNearTexture;
uniform sampler2D uDofFarTexture;
uniform float uStarburstOffset; // transforms texcoords
uniform bool uLensFlares;
uniform bool uBloom;
uniform bool uLensDirt = false;
uniform bool uDof = false;
uniform int uMotionBlur;
uniform float uBloomStrength = 0.1;
uniform float uBloomDirtStrength = 0.5;
uniform float uExposure = 1.0;
uniform float uVelocityScale;
uniform float uHalfPixelWidth = 0.0003125;

const float MAX_SAMPLES = 32.0;
const float SOFT_Z_EXTENT = 1.0;
const float Z_NEAR = 0.1;
const float Z_FAR = 3000.0;
const int TILE_SIZE = 40;
const float GAMMA = 1.5;

const float A = 0.15; // shoulder strength
const float B = 0.50; // linear strength
const float C = 0.10; // linear angle
const float D = 0.20; // toe strength
const float E = 0.02; // toe numerator
const float F = 0.30; // toe denominator
const vec3 W = vec3(11.2); // linear white point value

vec3 uncharted2Tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float zCompare(float a, float b)
{
	float result = (a - b) / min(a, b);
	return clamp(1.0 - result, 0.0, 1.0);
}

float softDepthCompare(float a, float b)
{
	return clamp(1.0 - (a - b) / SOFT_Z_EXTENT, 0.0, 1.0);
}

float cone(float dist, float velocityMag)
{
	return mix(0.0, clamp(1.0 - dist / velocityMag, 0.0, 1.0),  sign(velocityMag));
}

float cylinder(float dist, float velocityMag)
{
	return 1.0 - smoothstep(0.95 * velocityMag, 1.05 * velocityMag, dist);
}

float linearDepth(float depth)
{
    float z_n = 2.0 * depth - 1.0;
    return 2.0 * Z_NEAR * Z_FAR / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

const float HASHSCALE1 = 443.8975;

float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec2 jitteredNeighborMax()
{
	// TODO: do some actual jittering
	return texture(uVelocityNeighborMaxTexture, vTexCoord).rg;
}

void main()
{
	vec3 color = texture(uScreenTexture, vTexCoord).rgb;

	if (uMotionBlur == 1)
	{
		vec2 texelSize = 1.0/vec2(textureSize(uScreenTexture, 0));
	
		vec2 velocity = texture(uVelocityTexture, vTexCoord).rg;
		//velocity = pow(velocity, vec2(1.0 / 3.0));
		//velocity *= 2.0 * uVelocityScale;

		float speed = length(velocity / texelSize);
		float sampleCount = clamp(floor(speed), 1.0, MAX_SAMPLES);
		
		for (float i = 1; i < sampleCount; ++i) 
		{
			vec2 offset = velocity * (i / (sampleCount - 1) - 0.5);
			color += texture(uScreenTexture, vTexCoord + offset).rgb;
		}
		
		color /= sampleCount;
	}
	else if (uMotionBlur == 2)
	{
		vec2 neighborMaxVel = texture(uVelocityNeighborMaxTexture, vTexCoord).rg;
		float neighborMaxMag = length(neighborMaxVel);
		vec2 texSize = vec2(textureSize(uScreenTexture, 0));
		vec2 texelSize = 1.0 / texSize;

		if (neighborMaxMag > (texelSize.x * 0.5))
		{
			vec2 centerVelocity = texture(uVelocityTexture, vTexCoord).rg;
			float centerVelocityMag = length(centerVelocity);
			float weight = 1.0 / mix(1.0, centerVelocityMag, sign(centerVelocityMag));
			vec3 sum = color * weight;

			float centerDepth = -linearDepth(texture(uDepthTexture, vTexCoord).x);
			float rnd = clamp(hash12(vTexCoord), 0.0, 1.0) - 0.5;

			float numSamples = 25.0;

			for(float i = 0; i < numSamples; ++i)
			{
				// TODO: avoid this if
				if ( i == floor(numSamples * 0.5))
				{
					continue;
				}
				float t = mix(-1.0, 1.0, (i + rnd + 1.0) / (numSamples + 1.0));
				vec2 sampleCoord = vTexCoord + neighborMaxVel * t + (texelSize.x * 0.5);
				float sampleDepth = -linearDepth(texture(uDepthTexture, sampleCoord).x);

				float f = softDepthCompare(centerDepth, sampleDepth);
				float b = softDepthCompare(sampleDepth, centerDepth);

				float sampleVelocityMag = length(texture(uVelocityTexture, sampleCoord).rg);
				float dist = distance(sampleCoord, vTexCoord);

				float alpha = f * cone(dist, sampleVelocityMag)
							+ b * cone(dist, centerVelocityMag)
							+ cylinder(dist, sampleVelocityMag)
							* cylinder(dist, centerVelocityMag)
							* 2.0;

				weight += alpha;
				sum += alpha * texelFetch(uScreenTexture, ivec2(sampleCoord * texSize), 0).rgb;
			}

			color = sum / weight;
		}
	}
	else if (uMotionBlur == 3)
	{
		vec2 texSize = vec2(textureSize(uScreenTexture, 0));
		
		vec2 neighborhoodVelocity = jitteredNeighborMax() * texSize;
		float neighborhoodVelocityMag = max(length(neighborhoodVelocity), 0.5);

		if (neighborhoodVelocityMag > 0.5)
		{
			vec2 centerVelocity = texelFetch(uVelocityTexture, ivec2(gl_FragCoord.xy), 0).rg * texSize;
			float centerVelocityMag = max(length(centerVelocity), 0.5);
			
			vec2 wNeighborhood = normalize(neighborhoodVelocity);
			vec2 wPerpendicular = vec2(-wNeighborhood.y, wNeighborhood.x);
			
			if(dot(wPerpendicular, centerVelocity) < 0)
			{
				wPerpendicular = -wPerpendicular;
			}
			
			vec2 wCenter = normalize(mix(wPerpendicular, normalize(centerVelocity), (centerVelocityMag - 0.5) / GAMMA));

			const float sampleCount = 25.0;

			float totalWeight = sampleCount / (TILE_SIZE * centerVelocityMag);
			vec3 result = color * totalWeight;
			
			float centerDepth = -linearDepth(texelFetch(uDepthTexture, ivec2(gl_FragCoord.xy), 0).x);

			float rnd = 0.0;//clamp(hash12(vTexCoord), 0.0, 1.0) - 0.5;
			
			for(float i = 0; i < sampleCount; ++i)
			{
				float t = mix(-1.0 , 1.0, (i + rnd * 0.95 + 1.0) / (sampleCount + 1.0));
				
				vec2 samplingDir = mix(neighborhoodVelocity, centerVelocity, mod(i, 2.0));
				float dist = t * neighborhoodVelocityMag;
				ivec2 sampleCoord = ivec2(t * samplingDir) + ivec2(gl_FragCoord.xy);
				
				float sampleDepth = -linearDepth(texelFetch(uDepthTexture, sampleCoord, 0).x);
				
				float f = zCompare(centerDepth, sampleDepth);
				float b = zCompare(sampleDepth, centerDepth);
				
				vec2 sampleVelocity = texelFetch(uVelocityTexture, sampleCoord, 0).rg * texSize;
				
				float wA = dot(wCenter, samplingDir);
				float wB = dot(normalize(sampleVelocity), samplingDir);
				
				float sampleVelocityMag = max(length(sampleVelocity), 0.5);
				
				float weight = 0.0;
				weight += f * cone(dist, 1.0 / sampleVelocityMag) * wB;
				weight += b * cone(dist, 1.0 / centerVelocityMag) * wA;
				weight += cylinder(dist, min(sampleVelocityMag, centerVelocityMag)) * max(wA, wB) * 2.0;
				
				result += texelFetch(uScreenTexture, sampleCoord, 0).rgb * weight;
				totalWeight += weight;
			}

			color = result / totalWeight;
		}
	}

	if (uDof)
	{
		vec4 dofNear = texture(uDofNearTexture, vTexCoord);
		vec4 dofFar = texture(uDofFarTexture, vTexCoord);
		color = mix(color, dofFar.rgb, min(5.0 * dofFar.a, 0.5) * 2.0);
		color = mix(color, dofNear.rgb, min(5.0 * dofNear.a, 0.5) * 2.0);
	}


	if (uBloom)
	{
		vec3 lensMod = texture(uLensDirtTex, vTexCoord).rgb;
		vec3 bloom = texture(uBloomTexture, vTexCoord).rgb * uBloomStrength;
		//bloom = mix(bloom, bloom * (vec3(1.0) + lensMod), uBloomDirtStrength);
		color += bloom;
	}

	if (uLensFlares)
	{
		vec2 centerVec = vTexCoord - vec2(0.5);
		float d = length(centerVec);
		float radial = acos(centerVec.x / d);
		float mask = texture(uLensStarTex, vec2(radial + uStarburstOffset * 1.0, 0.0)).r
					* texture(uLensStarTex, vec2(radial - uStarburstOffset * 0.5, 0.0)).r;
		
		mask = clamp(mask + (1.0 - smoothstep(0.0, 0.3, d)), 0.0, 1.0);

		vec3 lensFlare = texture(uLensFlareTex, vTexCoord).rgb * mask;
		color.rgb += vec3(1.0) - exp(-lensFlare * 0.15 * dot(lensFlare, vec3(0.299, 0.587, 0.114)));
	}

	if (uLensDirt)
	{
		float brightness = dot(color.rgb, vec3(0.299, 0.587, 0.114));
		vec3 dirt = texture(uLensDirtTex, vTexCoord).rgb;
		color.rgb += (dirt + vec3(1.0)) * clamp(brightness / 5.0, 0.0, 1.0);
	}
	
	// HDR tonemapping
	float exposureBias = 2.0;
	//float exposure = 0.18/max((clamp(adaptedLum, -14.0, 16.0) - exposureBias), 0.00001);
	color *= uExposure;
	
	color = uncharted2Tonemap(exposureBias * color);
	vec3 whiteScale = 1.0/uncharted2Tonemap(W);
	color *= whiteScale;
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

	oFragColor = vec4(color, dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}