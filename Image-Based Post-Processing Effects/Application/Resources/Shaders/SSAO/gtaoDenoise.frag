#version 450 core

const float PI = 3.14159265;

in vec2 vTexCoord;

out vec4 oColor;

layout(binding=4) uniform sampler2D uVelocityTexture;
layout(binding=5) uniform sampler2D uPreviousTexture;
layout(binding=6) uniform sampler2D uInputTexture;

uniform int uFrame;

void main(void)
{
	vec2 center = texelFetch(uInputTexture, ivec2(gl_FragCoord.xy), 0).xy;
	float rampMaxInv = 1.0 / (center.y * 0.1);
	
	float totalAo = 0.0;
	float totalWeight = 0.0;
	
	int offset = 0;//uFrame % 2;
	for (int i = -1 - offset; i < 3 - offset; ++i)
	{
		for(int j = -2 + offset; j < 2 + offset; ++j)
		{
			vec2 S = texelFetch(uInputTexture, ivec2(gl_FragCoord.xy + vec2(i, j)), 0).xy;
			float weight = clamp(1.0 - (abs(S.y - center.y) * rampMaxInv), 0.0, 1.0);
			totalAo += S.x * weight;
			totalWeight += weight;
		}
	}
	
	float ao = totalAo / totalWeight;

	vec2 velocity = texture(uVelocityTexture, vTexCoord).rg;
	vec2 reprojectedCoord = vTexCoord - velocity;
	vec2 previousAo = texture(uPreviousTexture, reprojectedCoord).xy;

	float insideFrame = float(reprojectedCoord.x < 1.0 && reprojectedCoord.y < 1.0 && reprojectedCoord.x >= 0.0 && reprojectedCoord.y >= 0.0);
	float depthWeight = clamp(1.0 - (abs(previousAo.y - center.y) * rampMaxInv), 0.0, 1.0);
	ao = mix(ao, previousAo.x, (23.0 / 24.0) * depthWeight * insideFrame);
	
	oColor = vec4(ao, center.y, 0.0, 1.0);
}