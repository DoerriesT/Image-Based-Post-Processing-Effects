#version 450 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uCocTexture;


void main()
{
	vec2 texelSize = vec2(1.0 / textureSize(uCocTexture, 0));
	
	vec2 maxCoc = vec2(0.0);
	
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(-texelSize.x, texelSize.y)).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(0.0, texelSize.y)).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + texelSize).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(-texelSize.x, 0.0)).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(0.0, 0.0)).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(texelSize.x, 0.0)).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + -texelSize).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(0.0, -texelSize.y)).rg);
	maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + vec2(texelSize.x, -texelSize.y)).rg);
	
	oColor = vec4(maxCoc, 0.0, 0.0);
}