#version 330

layout(location = 0) out vec4 oAlbedo;
layout(location = 1) out vec4 oNormal;
layout(location = 2) out vec4 oMetallicRoughnessAo;

in vec3 vRay;

uniform samplerCube uAlbedoMap;
uniform vec4 uColor;
uniform bool uHasAlbedoMap;

void main()
{
    if (uHasAlbedoMap)
    {
        vec3 sample = texture(uAlbedoMap, normalize(vRay)).rgb;
		oAlbedo = vec4(sample * 0.1, 1.0);
    }
    else
    {
        oAlbedo = uColor;
    }
	oMetallicRoughnessAo = vec4(0.0);
	oNormal = vec4(0.0);
}