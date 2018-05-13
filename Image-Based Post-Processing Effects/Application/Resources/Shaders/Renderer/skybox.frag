#version 330

layout(location = 0) out vec4 oAlbedo;
layout(location = 1) out vec4 oNormal;
layout(location = 2) out vec4 oMetallicRoughnessAo;
layout(location = 3) out vec4 oVelocity;

in vec4 vRay;
in vec4 vCurrentPos;
in vec4 vPrevPos;

uniform samplerCube uAlbedoMap;
uniform vec4 uColor;
uniform bool uHasAlbedoMap;

void main()
{
    if (uHasAlbedoMap)
    {
		vec3 ray = vRay.xyz / vRay.w;
        vec3 sample = texture(uAlbedoMap, normalize(ray)).rgb;
		oAlbedo = vec4(sample * 0.1, 1.0);
    }
    else
    {
        oAlbedo = uColor;
    }
	oMetallicRoughnessAo = vec4(0.0);
	oNormal = vec4(0.0);

	vec2 a = (vCurrentPos.xy / vCurrentPos.w);
    vec2 b = (vPrevPos.xy / vPrevPos.w);
	vec2 v = abs(a - b);
	//v = pow(v, vec2(3.0));
	oVelocity = vec4(v, 0.0, 0.0);
}