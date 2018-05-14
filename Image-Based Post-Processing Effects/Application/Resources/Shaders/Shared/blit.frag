#version 330 core

out vec4 oFragColor;
in vec2 vTexCoord;

uniform sampler2D uScreenTexture;

void main()
{
	oFragColor = texture(uScreenTexture, vTexCoord).rgba;
}