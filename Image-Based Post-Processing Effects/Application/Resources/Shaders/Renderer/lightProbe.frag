#version 450 core

layout(location = 0) out vec4 oFragColor;

in vec3 vNormal;

layout(binding = 13) uniform samplerCube uLightProbe;

void main()
{
	oFragColor = vec4(textureLod(uLightProbe, vNormal, 0).rgb, 1.0);
}