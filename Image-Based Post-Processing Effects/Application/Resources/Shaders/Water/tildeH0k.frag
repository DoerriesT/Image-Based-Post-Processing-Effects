#version 450 core

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 oTildeH0k;
layout(location = 1) out vec4 oTildeH0minusK;

layout (pixel_center_integer) in vec4 gl_FragCoord;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uNoiseR0Texture;
layout(binding = 1) uniform sampler2D uNoiseI0Texture;
layout(binding = 2) uniform sampler2D uNoiseR1Texture;
layout(binding = 3) uniform sampler2D uNoiseI1Texture;

uniform int uN;
uniform int uL;
uniform float uA;
uniform vec2 uWindDirection;
uniform float uWindSpeed;
uniform float uWaveSuppressionExp = 4.0;

const float g = 9.81;

// Box-Muller-Method
vec4 gaussRND()
{
	float noise0 = clamp(texture(uNoiseR0Texture, vTexCoord).r, 0.001, 1.0);
	float noise1 = clamp(texture(uNoiseI0Texture, vTexCoord).r, 0.001, 1.0);
	float noise2 = clamp(texture(uNoiseR1Texture, vTexCoord).r, 0.001, 1.0);
	float noise3 = clamp(texture(uNoiseI1Texture, vTexCoord).r, 0.001, 1.0);
	
	float u0 = 2.0 * M_PI * noise0;
	float v0 = sqrt(-2.0 * log(noise1));
	float u1 = 2.0 * M_PI * noise2;
	float v1 = sqrt(-2.0 * log(noise3));
	
	vec4 rnd = vec4(v0 * cos(u0), v0 * sin(u0), v1 * cos(u1), v1 * sin(u1));
	
	return rnd;
}

void main() 
{
	vec2 x = gl_FragCoord.xy - float(uN) * 0.5;
	vec2 k = vec2(2.0 * M_PI * x.x/uL, 2.0 * M_PI * x.y/uL);
	
	float L_ = (uWindSpeed * uWindSpeed) / g;
	float mag = length(k);
	k /= mag;
	mag = max(mag, 0.00001);
	float magSq = mag * mag;

	float fraction = uL / 2000.0;
	float baseTerm = (uA / (magSq * magSq))
					* exp(-(1.0/(magSq * L_ * L_)))
					* exp(-magSq * fraction * fraction);

	// sqrt(Ph(k))/sqrt(2)
	float h0k = clamp(sqrt(pow(abs(dot(k, uWindDirection)), uWaveSuppressionExp) * baseTerm) / sqrt(2.0), -4000.0, 4000.0);

	// sqrt(Ph(-k))/sqrt(2)
	float h0minusK = clamp(sqrt(pow(abs(dot(-k, uWindDirection)), uWaveSuppressionExp) * baseTerm) / sqrt(2.0), -4000.0, 4000.0);
	
	vec4 gaussRandom = gaussRND();
	
	oTildeH0k = vec4(gaussRandom.xy * h0k, 0.0, 1.0);
	oTildeH0minusK = vec4(gaussRandom.zw * h0minusK, 0.0, 1.0);
}

