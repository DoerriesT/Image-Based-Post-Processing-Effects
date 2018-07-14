#version 450 core

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uScreenTexture;

uniform bool uDirection;

const float weights[] = float[]( 0.14107424, 0.242395713, 0.132279992, 0.044439883, 0.009181995, 0.00116529 );
const float offsets[] = float[]( 0.0, 1.5, 3.5, 5.5, 7.5, 9.5 );

void main()
{
	vec3 sum;
	vec2 texelSize = 1.0/vec2(textureSize(uScreenTexture, 0));
	if (uDirection)
	{
		sum += texture( uScreenTexture, vTexCoord).rgb * weights[0];
		for(int i = 1; i < 6; ++i)
		{
			sum += texture( uScreenTexture, vec2(vTexCoord.x, vTexCoord.y + offsets[i] * texelSize.y)).rgb * weights[i];
			sum += texture( uScreenTexture, vec2(vTexCoord.x, vTexCoord.y - offsets[i] * texelSize.y)).rgb * weights[i];
		}
	}
	else
	{
		sum += texture( uScreenTexture, vTexCoord).rgb * weights[0];
		for(int i = 1; i < 6; ++i)
		{
			sum += texture( uScreenTexture, vec2(vTexCoord.x + offsets[i] * texelSize.x, vTexCoord.y)).rgb * weights[i];
			sum += texture( uScreenTexture, vec2(vTexCoord.x - offsets[i] * texelSize.x, vTexCoord.y)).rgb * weights[i];
		}
	}

	oFragColor = vec4(sum, 1.0);
}