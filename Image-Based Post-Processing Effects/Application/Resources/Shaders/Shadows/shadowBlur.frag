#version 330 core

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uShadowMap;
uniform bool uDirection;
uniform int uWidth;
uniform int uHeight;

const float weights[] = float[]( 0.38774, 0.30613 );
const float offsets[] = float[]( 0.0, 1.5 );

void main()
{
	vec2 sum;
	if (uDirection)
	{
		float invHeight = 1.0/uHeight;
		sum += texture( uShadowMap, vTexCoord).rg * weights[0];
		for(int i = 1; i < 2; ++i)
		{
			sum += texture( uShadowMap, vec2(vTexCoord.x, vTexCoord.y + offsets[i] * invHeight)).rg * weights[i];
			sum += texture( uShadowMap, vec2(vTexCoord.x, vTexCoord.y - offsets[i] * invHeight)).rg * weights[i];
		}
	}
	else
	{
		float invWidth = 1.0/uWidth;
		sum += texture( uShadowMap, vTexCoord).rg * weights[0];
		for(int i = 1; i < 2; ++i)
		{
			sum += texture( uShadowMap, vec2(vTexCoord.x + offsets[i] * invWidth, vTexCoord.y)).rg * weights[i];
			sum += texture( uShadowMap, vec2(vTexCoord.x - offsets[i] * invWidth, vTexCoord.y)).rg * weights[i];
		}
	}

	oFragColor = vec4(sum, vec2(0.0, 1.0));
}