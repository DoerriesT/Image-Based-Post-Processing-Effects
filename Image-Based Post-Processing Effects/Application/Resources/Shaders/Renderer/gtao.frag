#version 450 core

const float PI = 3.14159265;

in vec2 vTexCoord;

out vec4 oColor;

layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 5) uniform sampler2D uNoiseMap;

uniform vec2 uFocalLength;
uniform mat4 uInverseProjection;

uniform vec2 uAORes = vec2(1600.0, 900.0);
uniform vec2 uInvAORes = vec2(1.0/1600.0, 1.0/900.0);
uniform vec2 uNoiseScale = vec2(1600.0, 900.0) / 4.0;

uniform float uStrength = 1.9;
uniform float uRadius = 0.3;
uniform float uMaxRadiusPixels = 50.0;

uniform float uNumDirections = 4;
uniform float uNumSteps = 4;


vec3 getViewSpacePos(vec2 uv)
{
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

float length2(vec3 v)
{
	return dot(v, v);
}

vec3 minDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (length2(V1) < length2(V2)) ? V1 : V2;
}

vec2 snapUVOffset(vec2 uv)
{
    return round(uv * uAORes) * uInvAORes;
}

vec2 rotateDirections(vec2 dir, vec2 cosSin)
{
    return vec2(dir.x * cosSin.x - dir.y * cosSin.y, dir.x * cosSin.y + dir.y * cosSin.x);
}

void computeSteps(inout vec2 stepSizeUv, inout float numSteps, float rayRadiusPix, float rand)
{
    // Avoid oversampling if numSteps is greater than the kernel radius in pixels
    numSteps = min(uNumSteps, rayRadiusPix);

    float stepSizePix = rayRadiusPix / numSteps;

    // Clamp numSteps if it is greater than the max kernel footprint
    float maxNumSteps = uMaxRadiusPixels / stepSizePix;
    if (maxNumSteps < numSteps)
    {
        // Use dithering to avoid AO discontinuities
        numSteps = floor(maxNumSteps + rand);
        numSteps = max(numSteps, 1);
        stepSizePix = uMaxRadiusPixels / numSteps;
    }

    // Step size in uv space
    stepSizeUv = stepSizePix * uInvAORes;
}

void main(void)
{
	vec3 P = getViewSpacePos(vTexCoord);

    // Get the random samples from the noise texture
	vec3 random = texture(uNoiseMap, vTexCoord.xy * uNoiseScale).rgb;

	// Calculate the projected size of the hemisphere
    vec2 rayRadiusUV = 0.5 * uRadius * uFocalLength / -P.z;
    float rayRadiusPix = rayRadiusUV.x * uAORes.x;

    float ao = 1.0;

    // Make sure the radius of the evaluated hemisphere is more than a pixel
    if(rayRadiusPix > 1.0)
    {
    	float numSteps;
    	vec2 stepSizeUV;

    	// Compute the number of steps
    	computeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z);

		// Sample neighboring pixels
		vec3 Pr = getViewSpacePos(vTexCoord + vec2( uInvAORes.x, 0));
		vec3 Pl = getViewSpacePos(vTexCoord + vec2(-uInvAORes.x, 0));
		vec3 Pt = getViewSpacePos(vTexCoord + vec2( 0, uInvAORes.y));
		vec3 Pb = getViewSpacePos(vTexCoord + vec2( 0,-uInvAORes.y));

		// Calculate tangent basis vectors using the minimu difference
		vec3 dPdu = minDiff(P, Pr, Pl);
		vec3 dPdv = minDiff(P, Pt, Pb) * (uAORes.y * uInvAORes.x);
		
		vec3 V = -normalize(P);
		vec3 N = normalize(cross(dPdu, dPdv));//decode(texture(uNormalMap, vTexCoord).rg);

		ao = 0.0;
		float alpha = PI / uNumDirections;

		// Calculate the horizon occlusion of each direction
		for(float d = 0; d < uNumDirections; ++d)
		{
			float theta = alpha * d;

			// Apply noise to the direction
			vec2 dir = rotateDirections(vec2(cos(theta), sin(theta)), random.xy);
			vec2 deltaUV = dir * stepSizeUV;
			
			vec2 horizons = vec2(-1.0);

			for(float s = 0; s < numSteps; ++s)
			{
				vec2 offset = (numSteps + 1.0) * deltaUV;
				
				// first horizon
				{
					vec3 S = getViewSpacePos(vTexCoord + offset);
					vec3 D = normalize(S - P);
					horizons.x = max(max(dot(V, D), 0.0), horizons.x);
				}
				
				// second horizon
				{
					vec3 S = getViewSpacePos(vTexCoord - offset);
					vec3 D = normalize(S - P);
					horizons.y = max(max(dot(V, D), 0.0), horizons.y);
				}
			}
			
			horizons = acos(horizons);
			horizons.x = -horizons.x;
			
			vec3 planeN = normalize(cross(V, vec3(dir, 0.0)));
			vec3 projectedN = N - (dot(N, planeN) / dot(planeN, planeN)) * planeN;
			float projectedNLength = length(projectedN);
			float invLength = 1.0 / (projectedNLength + 1e-6);
			projectedN *= invLength;
			
			float NdotV = dot(projectedN, V);
			float gamma = acos(NdotV);
			float sinGamma2 = sin(gamma) * 2.0;
			float cosGamma = cos(gamma);
			
			// clamp horizons
			horizons.x = gamma + max(horizons.x - gamma, -PI * 0.5);
			horizons.y = gamma + min(horizons.y - gamma, PI * 0.5);
			
			vec2 horizonCosTerm = (sinGamma2 * horizons - cos(2.0 * horizons - gamma)) + cosGamma;
			
			// premultiply
			projectedNLength *= 0.25;
			
			ao += projectedNLength * horizonCosTerm.x;
			ao += projectedNLength * horizonCosTerm.y;
		}

		// Average the results and produce the final AO
		ao = clamp(ao / uNumDirections, 0.0, 1.0);
	}

	oColor = vec4(pow(ao, uStrength), -P.z, 0.0, 0.0);
}