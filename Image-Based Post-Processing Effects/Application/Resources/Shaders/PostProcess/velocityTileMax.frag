#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

uniform sampler2D uVelocityTexture;
uniform bool uDirection;
uniform int uTileSize;

void vMax(vec2 velocity, inout vec2 maxVelocity, inout float maxMagnitude)
{
	float magnitude = length(velocity);
	if(magnitude > maxMagnitude)
	{
		maxMagnitude = magnitude;
		maxVelocity = velocity;
	}
}

void main()
{
	vec2 texelSize = vec2(1.0 / textureSize(uVelocityTexture, 0));
	
	if(uDirection)
	{
		texelSize.x = 0.0;
	}
	else
	{
		texelSize.y = 0.0;
	}
	
	vec2 maxVelocity = vec2(0.0);
	float maxMagnitude = 0.0;
	
	for(int i = uTileSize / -2; i < uTileSize / 2; ++i)
	{
		vMax(texture(uVelocityTexture, vTexCoord + i * texelSize).rg, maxVelocity, maxMagnitude);
	}
	
	oColor = vec4(maxVelocity, 0.0, 0.0);
}