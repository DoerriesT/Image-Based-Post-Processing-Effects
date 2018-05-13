#version 330

layout(location = 0) out vec4 oAlbedo;
layout(location = 1) out vec4 oVelocity;

in vec4 vRay;

uniform samplerCube uAlbedoMap;
uniform vec4 uColor;
uniform bool uHasAlbedoMap;
uniform mat4 uCurrentToPrevTransform;

void main()
{
	vec3 ray = vRay.xyz / vRay.w;
    if (uHasAlbedoMap)
    {	
        vec3 sample = texture(uAlbedoMap, normalize(ray)).rgb;
		oAlbedo = vec4(sample * 0.1, 1.0);
    }
    else
    {
        oAlbedo = uColor;
    }

	vec4 previous = uCurrentToPrevTransform * vec4(ray, 1.0);
	previous /= previous.w;

	vec2 v = abs(ray.xy - previous.xy);
	//v = pow(v, vec2(3.0));
	oVelocity = vec4(v, 0.0, 0.0);
}