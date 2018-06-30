#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uUpscaleTexture;
layout(binding = 1) uniform sampler2D uPreviousBlurredTexture;

uniform bool uAddPrevious;
uniform vec2 uRadius;


//const vec2 radius = vec2(0.1);

void main()
{

	//vec2 radius = vec2(1.0/textureSize(uUpscaleTexture, 0)) * 0.55;

	vec2 texCoord1 = vec2(-1.0, 1.0) * uRadius + vTexCoord;
	vec2 texCoord2 = vec2(0.0, 1.0) * uRadius + vTexCoord;
	vec2 texCoord3 = vec2(1.0, 1.0) * uRadius + vTexCoord;
	vec2 texCoord4 = vec2(-1.0, 0.0) * uRadius + vTexCoord;
	vec2 texCoord5 = vTexCoord;
	vec2 texCoord6 = vec2(1.0, 0.0) * uRadius + vTexCoord;
	vec2 texCoord7 = vec2(-1.0, -1.0) * uRadius + vTexCoord;
	vec2 texCoord8 = vec2(0.0, -1.0) * uRadius + vTexCoord;
	vec2 texCoord9 = vec2(1.0, -1.0) * uRadius + vTexCoord;

	vec4 sum = vec4(0.0);

	
	sum += texture(uUpscaleTexture, texCoord1).rgba;
	sum += texture(uUpscaleTexture, texCoord2).rgba * 2;
	sum += texture(uUpscaleTexture, texCoord3).rgba;
	sum += texture(uUpscaleTexture, texCoord4).rgba * 2;
	sum += texture(uUpscaleTexture, texCoord5).rgba * 4;
	sum += texture(uUpscaleTexture, texCoord6).rgba * 2;
	sum += texture(uUpscaleTexture, texCoord7).rgba;
	sum += texture(uUpscaleTexture, texCoord8).rgba * 2;
	sum += texture(uUpscaleTexture, texCoord9).rgba;

	sum /= 16.0;

	if (uAddPrevious)
	{
		sum += texture(uPreviousBlurredTexture, vTexCoord).rgba;
	}
	
	oColor = sum;
}