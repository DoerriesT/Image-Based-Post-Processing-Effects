#version 450 core

layout(location = 0) out vec4 oColor;

layout(early_fragment_tests) in;

uniform vec3 uColor;

void main()
{
	oColor = vec4(uColor, 1.0);
}