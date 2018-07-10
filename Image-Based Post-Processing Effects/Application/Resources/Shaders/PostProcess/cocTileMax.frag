#version 450 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uCocTexture;

uniform bool uDirection;
uniform int uTileSize;


void main()
{
	vec2 texelSize = vec2(1.0 / textureSize(uCocTexture, 0));
	
	if(uDirection)
	{
		texelSize.x = 0.0;
	}
	else
	{
		texelSize.y = 0.0;
	}
	
	vec2 maxCoc = vec2(0.0);
	
	for(int i = uTileSize / -2; i < uTileSize / 2; ++i)
	{
		maxCoc = max(maxCoc, texture(uCocTexture, vTexCoord + i * texelSize).rg);
	}
	
	oColor = vec4(maxCoc, 0.0, 0.0);
}