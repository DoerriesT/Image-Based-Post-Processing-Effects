#version 450 core

const float PI = 3.14159265;

in vec2 vTexCoord;

out vec4 oColor;

layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 5) uniform sampler2D uNoiseMap;

uniform float uFocalLength;
uniform mat4 uInverseProjection;

uniform vec2 uAORes = vec2(1600.0, 900.0);
uniform vec2 uInvAORes = vec2(1.0/1600.0, 1.0/900.0);

uniform float uStrength = 1.9;
uniform float uRadius = 0.3;
uniform float uMaxRadiusPixels = 50.0;

uniform float uNumSteps = 4;

uniform int uFrame;


vec3 getViewSpacePos(vec2 uv)
{
	uv *= uInvAORes;
	float depth = texture(uDepthMap, uv).r;
	vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;
	//viewSpacePosition.z = -viewSpacePosition.z;
	return viewSpacePosition.xyz;
}

vec3 decode (vec2 enc)
{
    vec2 fenc = enc * 4.0 - 2.0;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0 - f * 0.25);
    vec3 n;
    n.xy = fenc * g;
    n.z = 1.0 -f * 0.5;
    return n;
}

vec3 minDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (dot(V1, V1) < dot(V2, V2)) ? V1 : V2;
}

void computeSteps(inout float stepSizePix, inout float numSteps, float rayRadiusPix)
{
    // Avoid oversampling if numSteps is greater than the kernel radius in pixels
    numSteps = min(uNumSteps, rayRadiusPix);
    stepSizePix = rayRadiusPix / numSteps;
}

// x = spatial direction / y = temporal direction / z = spatial offset / w = temporal offset
vec4 getNoise()
{
	vec4 noise;
	
	ivec2 coord = ivec2(gl_FragCoord.xy);
	
	noise.x = (1.0 / 16.0) * ((((coord.x + coord.y) & 0x3 ) << 2) + (coord.x & 0x3));
	noise.z = (1.0 / 4.0) * ((coord.y - coord.x) & 0x3);

	float rotations[] = { 60.0, 300.0, 180.0, 240.0, 120.0, 0.0 };
	noise.y = rotations[uFrame % 6] * (1.0 / 360.0);
	
	float offsets[] = { 0.0, 0.5, 0.25, 0.75 };
	noise.w = offsets[(uFrame / 6 ) % 4];
	
	return noise;
}

float square(float x)
{
	return x * x;
}

float falloff(float dist2)
{
	float start = square(uRadius * 0.2);
	float end = square(uRadius);
	return 2.0 * clamp((dist2 - start) / (end - start), 0.0, 1.0);
}

void main(void)
{
	vec3 P = getViewSpacePos(gl_FragCoord.xy);

	// Calculate the projected size of the hemisphere
    float rayRadiusUV = 0.5 * uRadius * uFocalLength / -P.z;
    float rayRadiusPix = rayRadiusUV * uAORes.x;
	rayRadiusPix = min(rayRadiusPix, uMaxRadiusPixels);

    float ao = 1.0;

    // Make sure the radius of the evaluated hemisphere is more than a pixel
    if(rayRadiusPix > 1.0)
    {
		ao = 0.0;
		
		// Sample neighboring pixels
		vec3 Pr = getViewSpacePos(gl_FragCoord.xy + vec2(1.0, 0.0));
		vec3 Pl = getViewSpacePos(gl_FragCoord.xy + vec2(-1.0, 0.0));
		vec3 Pt = getViewSpacePos(gl_FragCoord.xy + vec2(0.0, 1.0));
		vec3 Pb = getViewSpacePos(gl_FragCoord.xy + vec2(0.0, -1.0));

		// Calculate tangent basis vectors using the minimum difference
		vec3 dPdu = minDiff(P, Pr, Pl);
		vec3 dPdv = minDiff(P, Pt, Pb);
		
		vec3 N = normalize(cross(dPdu, dPdv));//decode(texture(uNormalMap, vTexCoord).rg);
		
		vec3 V = -normalize(P);
		
		float numSteps;
    	float stepSizePix;

    	// Compute the number of steps
    	computeSteps(stepSizePix, numSteps, rayRadiusPix);

		vec4 noise = getNoise();

		float theta = (noise.x + noise.y) * PI;
		float jitter = noise.z + noise.w;
		vec2 dir = vec2(cos(theta), sin(theta));
		vec2 horizons = vec2(-1.0);
		
		float currstep = mod(jitter, 1.0) * (stepSizePix - 1.0) + 1.0;

		for(float s = 0; s < numSteps; ++s)
		{
			vec2 offset = currstep * dir;
			currstep += stepSizePix;
			
			// first horizon
			{
				vec3 S = getViewSpacePos(gl_FragCoord.xy + offset);
				vec3 D = S - P;
				float dist2 = dot(D, D);
				D *= inversesqrt(dist2);
				float attenuation = falloff(dist2);
				horizons.x = max(dot(V, D) - attenuation, horizons.x);
			}
			
			// second horizon
			{
				vec3 S = getViewSpacePos(gl_FragCoord.xy - offset);
				vec3 D = S - P;
				float dist2 = dot(D, D);
				D *= inversesqrt(dist2);
				float attenuation = falloff(dist2);
				horizons.y = max(dot(V, D) - attenuation, horizons.y);
			}
		}
		
		horizons = acos(horizons);
		horizons.x = -horizons.x;
		
		// project normal onto slice plane
		vec3 planeN = normalize(cross(vec3(dir, 0.0), V));
		vec3 projectedN = N - dot(N, planeN) * planeN;
		
		float projectedNLength = length(projectedN);
		float invLength = 1.0 / (projectedNLength + 1e-6);
		projectedN *= invLength;
		
		// calculate gamma
		vec3 tangent = cross(V, planeN);
		float cosGamma	= dot(projectedN, V);
		float gamma = acos(cosGamma) * sign(-dot(projectedN, tangent));
		float sinGamma2	= 2.0 * sin(gamma);
		
		
		// clamp horizons
		horizons.x = gamma + max(horizons.x - gamma, -PI * 0.5);
		horizons.y = gamma + min(horizons.y - gamma, PI * 0.5);
		
		vec2 horizonCosTerm = (sinGamma2 * horizons - cos(2.0 * horizons - gamma)) + cosGamma;
		
		// premultiply
		projectedNLength *= 0.25;
		
		ao += projectedNLength * horizonCosTerm.x;
		ao += projectedNLength * horizonCosTerm.y;
	}

	oColor = vec4(ao, -P.z, 0.0, 0.0);
}