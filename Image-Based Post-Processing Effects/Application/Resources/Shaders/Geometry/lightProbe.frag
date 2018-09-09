#version 450 core

layout(location = 0) out vec4 oFragColor;

layout(early_fragment_tests) in;

in vec3 vNormal;

layout(binding = 13) uniform sampler2D uLightProbe;

uniform bool uSH;

uniform int uIndex;

void main()
{
	const vec3 N = normalize(vNormal);
	vec3 irradiance = vec3(0.0);
	irradiance += texelFetch(uLightProbe, ivec2(0, uIndex), 0).rgb * 0.282095f;
	irradiance += texelFetch(uLightProbe, ivec2(1, uIndex), 0).rgb * 0.488603f * N.y;
	irradiance += texelFetch(uLightProbe, ivec2(2, uIndex), 0).rgb * 0.488603f * N.z;
	irradiance += texelFetch(uLightProbe, ivec2(3, uIndex), 0).rgb * 0.488603f * N.x;
	irradiance += texelFetch(uLightProbe, ivec2(4, uIndex), 0).rgb * 1.092548f * N.x * N.y;
	irradiance += texelFetch(uLightProbe, ivec2(5, uIndex), 0).rgb * 1.092548f * N.y * N.z;
	irradiance += texelFetch(uLightProbe, ivec2(6, uIndex), 0).rgb * 0.315392f * (3.0f * N.z * N.z - 1.0f);
	irradiance += texelFetch(uLightProbe, ivec2(7, uIndex), 0).rgb * 1.092548f * N.x * N.z;
	irradiance += texelFetch(uLightProbe, ivec2(8, uIndex), 0).rgb * 0.546274f * (N.x * N.x - N.y * N.y);
	oFragColor = vec4(irradiance, 1.0);
}