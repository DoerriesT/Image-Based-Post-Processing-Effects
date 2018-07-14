#version 450 core

#define textureThreshold(tex, texCoord) (max(vec4(0.0), texture(tex, texCoord) + uBias) * uScale)
#define textureDistorted(tex, texCoord, direction, distortion) (vec3(textureThreshold(tex, texCoord + direction * distortion.r).r,textureThreshold(tex, texCoord + direction * distortion.g).g,textureThreshold(tex, texCoord + direction * distortion.b).b))

out vec4 oFragColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uInputTex;
layout(binding = 1) uniform sampler2D uLensColor;

uniform int uGhosts; // number of ghost samples
uniform float uGhostDispersal; // dispersion factor
uniform float uHaloRadius;
uniform float uHaloThickness = 0.1;
uniform float uDistortion;
uniform float uAspectRatio = (1600.0 / 900.0);
uniform vec4 uScale = vec4(vec3(0.5), 1.0);
uniform vec4 uBias = vec4(-32.0);

/*
vec3 textureDistorted(in sampler2D tex, in vec2 texCoord, in vec2 direction, in vec3 distortion) 
{
    return vec3(
        textureThreshold(tex, texCoord + direction * distortion.r).r,
        textureThreshold(tex, texCoord + direction * distortion.g).g,
        textureThreshold(tex, texCoord + direction * distortion.b).b
    );
}
*/

void main() 
{
	vec2 texCoord = vec2(1.0) - vTexCoord;
    
	// ghost vector to image centre
    vec2 ghostVec = (vec2(0.5) - texCoord) * uGhostDispersal;
   
    vec3 result = vec3(0.0);
	
	// sample ghosts
	{
		for (int i = 0; i < uGhosts; ++i) 
		{ 
			vec2 sampleCoord = fract(vec2(i) * ghostVec + texCoord);
			
			float distanceToCenter = distance(sampleCoord, vec2(0.5));
			float weight = 1.0 - smoothstep(0.0, 0.75, distanceToCenter);
	
			result += textureThreshold(uInputTex, sampleCoord).rgb * weight;
		}

		result *= texture(uLensColor, vec2(distance(texCoord, vec2(0.5)), 0.5), 0.0).rgb;
	}
	
	
	// sample halo
	{
		// calculate weight
		float weight = distance(texCoord, vec2(0.5));
		weight = min(abs(weight - uHaloRadius) / uHaloThickness, 1.0);
		weight = 1.0 - weight * weight * (3.0 - 2.0 * weight);
		
		vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
		vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);
		vec2 direction = normalize(ghostVec);
		vec2 haloVec = direction;
		
		//haloVec.x *= uAspectRatio;
		//haloVec = normalize(haloVec);
		//haloVec.x /= uAspectRatio;
		//vec2 wuv = (texCoord - vec2(0.5, 0.0)) / vec2(uAspectRatio, 1.0) + vec2(0.5, 0.0);
		//float weight = distance(wuv, vec2(0.5));
		
		haloVec *= uHaloRadius;
		
		result.rgb += textureDistorted(uInputTex, texCoord + haloVec, direction, distortion) * weight;
	}

    oFragColor = vec4(result * 0.03, 1.0);
}