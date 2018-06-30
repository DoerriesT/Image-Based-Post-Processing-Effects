#version 450 core

//#define SEPIA

out vec4 oFragColor;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uScreenTexture;

uniform float uChromAbOffsetMultiplier;
uniform float uTime;
uniform float uFilmGrainStrength;
uniform bool uChromaticAberration;
uniform bool uVignette;
uniform bool uFilmGrain;

const float HASHSCALE1 = 443.8975;
const vec2 CENTER = vec2(0.5, 0.5);

// radius of vignette, where 0.5 results in a circle fitting the screen
const float RADIUS = 0.75;
// softness of vignette, between 0.0 and 1.0
const float SOFTNESS = 0.45;

const vec3 SEPIA_COEFFICIENT = vec3(1.2, 1.0, 0.8);

float hash13(vec3 p3)
{
	p3  = fract(p3 * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

void main()
{
	float distanceToCenter = length(vTexCoord - CENTER);

	vec3 color = vec3(0.0);

	if (uChromaticAberration)
	{
		float offSet = distanceToCenter * uChromAbOffsetMultiplier;
		color = vec3(texture(uScreenTexture, vTexCoord + offSet).r, texture(uScreenTexture, vTexCoord).g, texture(uScreenTexture, vTexCoord - offSet).b);
	}
	else
	{
		color = texture(uScreenTexture, vTexCoord).rgb;
	}

	if (uVignette)
	{
		float distanceToCenter = length(vTexCoord - CENTER);
		float vignette = smoothstep(RADIUS, RADIUS-SOFTNESS, distanceToCenter);
		color = mix(color, color * vignette, 0.5);
	}
    
	if (uFilmGrain)
	{
		float h = hash13( vec3(vTexCoord, mod(uTime, 100.0)) );
		color *= (h * 2.0 - 1.0) * uFilmGrainStrength + (1.0f - uFilmGrainStrength);
	}

#ifdef SEPIA
	float gray = dot(color, vec3(0.299, 0.587, 0.114));
	color = vec3(gray) * SEPIA_COEFFICIENT;
#endif // SEPIA

	oFragColor = vec4(color, 1.0);
}