#version 450 core

layout(location = 0) out vec4 oDiffuse;
layout(location = 1) out vec4 oNormal;

in vec2 vTexCoord;
in vec3 vNormal;

layout(binding = 0) uniform sampler2D uAlbedoMap;

uniform bool uHasTexture;
uniform vec3 uAlbedo;
uniform vec3 uLightColor;
uniform vec3 uLightDir;

void main()
{
	vec3 albedo = uHasTexture ? texture(uAlbedoMap, vTexCoord).rgb : uAlbedo;
	vec3 N = normalize(vNormal);
	oDiffuse = vec4(albedo * uLightColor * max(0.0, dot(N, uLightDir)), 1.0);
	oNormal = vec4(N, gl_FragCoord.z);
}