#version 450 core

layout(location = 2) out vec4 oAlbedo;

in vec3 teWorldPos;

uniform vec3 uCamPos;
uniform vec3 uLightIntensity;
uniform vec3 uSigmaExtinction;
uniform vec3 uScatterPower;
uniform vec3 uLightDir;
uniform sampler2D uPhaseLUT;

const float PI = 3.1415926535898;

vec3 GetPhaseFactor(float cos_theta)
{
    vec2 tc;
    tc.x = 0.0;
    tc.y = acos(clamp(-cos_theta, -1.0, 1.0)) / PI;
    return uScatterPower * textureLod(uPhaseLUT, tc, 0.0).rgb;
}

vec3 Integrate_SimpleDirectional(float eye_dist, vec3 vV, vec3 vL)
{
    // Do basic directional light
    float VdotL = dot(normalize(vV), normalize(vL));
    vec3 sigma = uSigmaExtinction;
    return GetPhaseFactor(-VdotL) * (1.0 - exp(-sigma * eye_dist)) / (sigma);
}

void main()
{
	float fSign = gl_FrontFacing ? -1.0f : 1.0f;
    vec3 vWorldPos = teWorldPos;
    float eye_dist = length(vWorldPos.xyz - uCamPos.xyz);
    vec3 vV = (vWorldPos.xyz - uCamPos.xyz) / eye_dist;
	
	vec3 integral = Integrate_SimpleDirectional(eye_dist, vV, uLightDir);

	oAlbedo = vec4(fSign * integral * uLightIntensity.rgb, 1.0);
}

