#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

uniform sampler2D uVelocityTexture;

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
	vec2 texCoord = vTexCoord * 20.0;
	
	vec2 maxVelocity = vec2(0.0);
	float maxMagnitude = 0.0;
	
	vMax(texture(uVelocityTexture, texCoord + vec2(-texelSize.x, texelSize.y)).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + vec2(0.0, texelSize.y)).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + texelSize).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + vec2(-texelSize.x, 0.0)).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + vec2(0.0, 0.0)).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + vec2(texelSize.x, 0.0)).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + -texelSize).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + vec2(0.0, -texelSize.y)).rg, maxVelocity, maxMagnitude);
	vMax(texture(uVelocityTexture, texCoord + vec2(texelSize.x, -texelSize.y)).rg, maxVelocity, maxMagnitude);
	
	oColor = vec4(maxVelocity, 0.0, 0.0);
}