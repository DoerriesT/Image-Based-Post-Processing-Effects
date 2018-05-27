#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

uniform sampler2D uDepthTexture;
uniform sampler2D uNoiseTexture;

const float Z_NEAR = 0.1;
const float Z_FAR = 3000.0;

float linearDepth(float depth)
{
    float z_n = 2.0 * depth - 1.0;
    return 2.0 * Z_NEAR * Z_FAR / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

void main()
{
	// get rotation vector, rotation is tiled every 4 pixels
	vec2 texSize = textureSize(uDepthTexture, 0);
	vec2 rotationTexCoord = vTexCoord * texSize * 0.25;
	vec3 vRotation = texture(uNoiseTexture, rotationTexCoord).rgb;
	
	// create rotation matrix from rotation vector
	mat3 rotMat;
	float h = 1.0 / (1.0 + vRotation.z);
	rotMat[0].x = h * vRotation.y * vRotation.y + vRotation.z;
	rotMat[0].y = -h * vRotation.y * vRotation.x;
	rotMat[0].z = -vRotation.x;
	rotMat[1].x = -h * vRotation.y * vRotation.x;
	rotMat[1].y = h * vRotation.x * vRotation.x + vRotation.z;
	rotMat[1].z = -vRotation.y;
	rotMat[2] = vRotation;
	rotMat = transpose(rotMat);
	
	float centerDepth = linearDepth(texture(uDepthTexture, vTexCoord).x);
	
	// parameters affecting offset points number and distribution
	const int numSamples = 16;
	float offsetScale = 0.01;
	const float offsetScaleStep = 1.0 + 2.4 / numSamples;
	
	float accessibility = 0.0;
	
	// sample area and accumulate accessiblity
	for (int i = 0; i < (numSamples / 8); ++i)
	{
		for (int x = -1; x < 2; x += 2)
		{
			for (int y = -1; y < 2; y += 2)
			{
				for (int z = -1; z < 2; z += 2)
				{
					// generate offset vector
					// here we use cube corners and give it different 
					offsetScale *= offsetScaleStep;
					vec3 vOffset = normalize(vec3(x, y, z)) * offsetScale;
					
					// rotate offset vector by rotation matrix
					vec3 vRotatedOffset =  vOffset;// * rotMat;
					
					// get center pixel 3d coordinates in screen space
					vec3 vSamplePos = vec3(vTexCoord, centerDepth);
					
					// shift coordinates by offset vector (range convert and width depth value)
					//vSamplePos += vec3(vRotatedOffset.xy, vRotatedOffset.z * centerDepth * 2.0);
					vSamplePos += vRotatedOffset;
					
					// read scene depth at sampling point and convert into meters
					float sampleDepth = linearDepth(texture(uDepthTexture, vSamplePos.xy).x);
					
					// check if depths of both pixels are close enough and sampling point
					// should affect our center pixel
					float rangeIsInvalid = clamp((centerDepth - sampleDepth) / sampleDepth, 0.0, 1.0);
					
					// accumulate accessibility, use default value of 0.5 if right computations are not possible
					accessibility += mix(float(sampleDepth > vSamplePos.z), 0.5, rangeIsInvalid);
				}
			}
		}
	}
	
	// get average value
	accessibility /= numSamples;
	
	// amplify and saturate if necessary
	oColor = vec4(clamp(accessibility * accessibility + accessibility, 0.0, 1.0), 0.0, 0.0, 0.0);
}