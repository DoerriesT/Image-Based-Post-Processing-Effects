#version 330 core

layout(location = 0) out vec4 oFragColor;

in vec4 vWorldPos;
in vec3 vNormal;

uniform vec3 uLightDirection;
uniform vec3 uCamPos;

float phase(float alpha, float g)
{
    float a = 3.0 * (1.0 - g * g);
    float b = 2.0 * (2.0 + g * g);
    float c = 1.0 + alpha * alpha;
    float d = pow(1.0 + g * g - 2.0 * g * alpha, 1.5);
    return (a / b) * (c / d);
}

void main()
{
	vec3 V = normalize(uCamPos - vWorldPos.xyz/vWorldPos.w);
	vec3 N = normalize(vNormal);

	float NdotL = clamp(dot(N, uLightDirection), 0.0, 1.0);
	float NdotV = clamp(dot(N, V), 0.0, 1.0);

	vec4 color = phase(NdotL, 0.9995) * vec4(1000.0);

	oFragColor = vec4(mix(vec4(0.0), vec4(vec3(0.3, 0.5, 2.5) * 20.0, 7), smoothstep(0.0, 1.0, pow(abs(1.0 - NdotV - 0.1), 6))) * NdotL);
}