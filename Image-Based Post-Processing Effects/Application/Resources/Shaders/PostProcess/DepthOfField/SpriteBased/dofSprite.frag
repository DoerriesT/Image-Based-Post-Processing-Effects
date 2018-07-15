#version 450 core

layout(location = 0) out vec4 oFragColor;

layout(binding = 4) uniform sampler2D uSpriteTexture;

in flat float vNear;
in flat vec4 vColor;
in vec2 vTexCoord;

uniform int uWidth;

void main()
{
	vec4 spriteColor = texture(uSpriteTexture, vTexCoord).rgba;
	oFragColor = vec4(vColor.rgb * spriteColor.rgb, spriteColor.a);// * vColor.a;
	
	bool leftSide = (gl_FragCoord.x < uWidth);
	float borderAlpha = float(bool(vNear) == leftSide);
	oFragColor *= borderAlpha;
}