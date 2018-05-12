#version 330 core

#define textureThreshold(tex, texCoord) (max(vec4(0.0), texture(tex, texCoord) + uBias) * uScale)
#define textureDistorted(tex, texCoord, direction, distortion) (vec3(textureThreshold(tex, texCoord + direction * distortion.r).r,textureThreshold(tex, texCoord + direction * distortion.g).g,textureThreshold(tex, texCoord + direction * distortion.b).b))

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uInputTex;
uniform sampler2D uLensColor;
uniform int uGhosts; // number of ghost samples
uniform float uGhostDispersal; // dispersion factor
uniform float uHaloWidth;
uniform float uDistortion;
uniform vec4 uScale = vec4(vec3(2.0), 1.0);
uniform vec4 uBias = vec4(-6.0);

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
	vec2 texCoord = -vTexCoord + vec2(1.0);
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
 
	// ghost vector to image centre:
    vec2 ghostVec = (vec2(0.5) - texCoord) * uGhostDispersal;
	
	vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);
	vec2 direction = normalize(ghostVec);
   
	// sample ghosts:  
    vec4 result = vec4(0.0);
	for (int i = 0; i < uGhosts; ++i) 
	{ 
		vec2 offset = fract(texCoord + ghostVec * float(i));
      
		float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
		weight = pow(1.0 - weight, 10.0);
  
		result += textureThreshold(uInputTex, offset) * weight;
	}
	result *= texture(uLensColor, vec2(length(vec2(0.5) - vTexCoord) / length(vec2(0.5)), 0.0));
	
	// sample halo:
	vec2 haloVec = normalize(ghostVec) * uHaloWidth;
	float weight = length(vec2(0.5) - fract(texCoord + haloVec)) / length(vec2(0.5));
	weight = pow(1.0 - weight, 5.0);
	result.rgb += textureDistorted(uInputTex, texCoord + haloVec, direction, distortion) * weight;
	
    oFragColor = result * 0.03;
}