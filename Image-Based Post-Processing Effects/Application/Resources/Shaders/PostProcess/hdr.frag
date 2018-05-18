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
uniform mat3 uLensStarMatrix; // transforms texcoords
uniform bool uLensFlares;
uniform bool uBloom;
uniform int uMotionBlur;
uniform float uBloomStrength = 0.1;
uniform float uBloomDirtStrength = 0.5;
uniform float uExposure = 1.0;
uniform float uVelocityScale;

const float MAX_SAMPLES = 32.0;
const float SOFT_Z_EXTENT = 1.0;
const float Z_NEAR = 0.1;
const float Z_FAR = 3000.0;

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

float softDepthCompare(float a, float b)
{
	return clamp(1.0 - (a - b) / SOFT_Z_EXTENT, 0.0, 1.0);
}

float cone(vec2 x, vec2 y, float velocityMag)
{
	return mix(0.0, clamp(1.0 - length(x - y) / velocityMag, 0.0, 1.0),  sign(velocityMag));
}

float cylinder(vec2 x, vec2 y, float velocityMag)
{
	return 1.0 - smoothstep(0.95 * velocityMag, 1.05 * velocityMag, length(x - y));
}

float linearDepth(vec2 coord)
{
	float z_b = texture(uDepthTexture, coord).x;
    float z_n = 2.0 * z_b - 1.0;
    return 2.0 * Z_NEAR * Z_FAR / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

const float HASHSCALE1 = 443.8975;

float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
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

			float centerDepth = -linearDepth(vTexCoord);
			float rnd = clamp(hash12(vTexCoord), 0.0, 1.0) - 0.5;

			for(int i = 0; i < 19; ++i)
			{
				// TODO: avoid this if
				if ( i == 9)
				{
					continue;
				}
				float t = mix(-1.0, 1.0, (i + rnd + 1.0) / 20.0);
				vec2 sampleCoord = vTexCoord + neighborMaxVel * t + (texelSize.x * 0.5);
				float sampleDepth = -linearDepth(sampleCoord);

				float f = softDepthCompare(centerDepth, sampleDepth);
				float b = softDepthCompare(sampleDepth, centerDepth);

				float sampleVelocityMag = length(texture(uVelocityTexture, sampleCoord).rg);

				float alpha = f * cone(sampleCoord, vTexCoord, sampleVelocityMag)
							+ b * cone(vTexCoord, sampleCoord, centerVelocityMag)
							+ cylinder(sampleCoord, vTexCoord, sampleVelocityMag)
							* cylinder(vTexCoord, sampleCoord, centerVelocityMag)
							* 2.0;

				weight += alpha;
				sum += alpha * texelFetch(uScreenTexture, ivec2(sampleCoord * texSize), 0).rgb;
			}

			color = sum / weight;
		}
	}


	if (uBloom)
	{
		vec3 lensMod = texture(uLensDirtTex, vTexCoord).rgb;
		vec3 bloom = texture(uBloomTexture, vTexCoord).rgb * uBloomStrength;
		bloom = mix(bloom, bloom * (vec3(1.0) + lensMod), uBloomDirtStrength);
		color += bloom;
	}

	if (uLensFlares)
	{
		vec3 lensMod = texture(uLensDirtTex, vTexCoord).rgb;
		vec2 lensStarTexCoord = (uLensStarMatrix * vec3(vTexCoord, 1.0)).xy;
		lensStarTexCoord = clamp(lensStarTexCoord, vec2(0.0), vec2(1.0));
		lensMod += texture(uLensStarTex, lensStarTexCoord).rgb;
		vec3 lensFlare = texture(uLensFlareTex, vTexCoord).rgb * lensMod;
		color.rgb += vec3(1.0) - exp(-lensFlare * 8.0 * dot(lensFlare, vec3(0.299, 0.587, 0.114)));
		//color.rgb += lensFlare;
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