#version 450 core

layout(location = 0) out vec4 oColor;

layout(early_fragment_tests) in;

uniform vec4 uOutlineColor;

void main()
{
	oColor = uOutlineColor;
}

