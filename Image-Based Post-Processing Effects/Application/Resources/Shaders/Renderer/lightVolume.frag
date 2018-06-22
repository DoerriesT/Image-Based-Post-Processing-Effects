#version 450 core

layout(location = 0) out vec4 oAlbedo;

in vec3 teWorldPos;

uniform vec3 uCamPos;
uniform vec3 uLightIntensity;
uniform vec3 uSigmaExtinction;
uniform vec3 uScatterPower;
uniform vec3 uLightDir;
uniform sampler2D uPhaseLUT;
uniform sampler2D uDepthTexture;
uniform mat4 uViewProjection;
uniform mat4 uInvViewProjection;
uniform int uPassMode;
uniform float uZFar;

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
    float VdotL = dot(vV, vL);
    vec3 sigma = uSigmaExtinction;
    return GetPhaseFactor(-VdotL) * (1.0 - exp(-sigma * eye_dist)) / (sigma);
}

void main()
{
	float fSign = 0.0;
    vec4 vWorldPos = vec4(0.0);
    float eye_dist = 0.0;
    vec3 vV = vec3(0.0);

	// geometry
	if (uPassMode == 0)
	{
		fSign = gl_FrontFacing ? -1.0 : 1.0;
		vWorldPos = vec4(teWorldPos, 1.0);
		eye_dist = length(vWorldPos.xyz - uCamPos);
		vV = (vWorldPos.xyz - uCamPos.xyz) / eye_dist;
	}
	// sky
	else if (uPassMode == 1)
	{
		fSign = 1.0f;
        eye_dist = uZFar;
        vV = normalize(teWorldPos - uCamPos);
	}
	// final
	else
	{
		fSign = 1.0f;
		vec4 projectedPos = uViewProjection * vec4(teWorldPos, 1.0);
		projectedPos.xy *= 1.0 / projectedPos.w;
        float fSceneDepth = textureLod(uDepthTexture, projectedPos.xy * 0.5 + 0.5, 0).x;
        projectedPos.zw = vec2(fSceneDepth, 1.0);
        vWorldPos = uInvViewProjection * projectedPos;
        vWorldPos *= 1.0f / vWorldPos.w;
        eye_dist = length(vWorldPos.xyz - uCamPos.xyz);
        vV = (vWorldPos.xyz - uCamPos.xyz) / eye_dist;
	}
	
	vec3 integral = Integrate_SimpleDirectional(eye_dist, vV, uLightDir);

	vec3 color = fSign * integral * uLightIntensity.rgb;

	oAlbedo = vec4(color, 1.0);
}

