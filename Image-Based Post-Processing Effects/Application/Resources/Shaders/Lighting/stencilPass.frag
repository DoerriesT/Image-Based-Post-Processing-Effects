#version 450 core

out vec4 oFragColor;  

layout(early_fragment_tests) in;

void main()
{
	oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
}