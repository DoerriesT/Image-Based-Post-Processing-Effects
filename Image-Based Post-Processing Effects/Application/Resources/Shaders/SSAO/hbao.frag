#version 450 core

const float PI = 3.14159265;

in vec2 vTexCoord;

out vec4 oColor;

layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 5) uniform sampler2D uNoiseMap;

uniform vec2 uFocalLength;
uniform mat4 uInverseProjection;

uniform vec2 uAORes = vec2(1600.0, 900.0);
uniform vec2 uInvAORes = vec2(1.0/1600.0, 1.0/900.0);
uniform vec2 uNoiseScale = vec2(1600.0, 900.0) / 4.0;

uniform float uStrength = 1.9;
uniform float uRadius = 0.3;
uniform float uRadius2 = 0.3*0.3;
uniform float uNegInvR2 = - 1.0 / (0.3*0.3);
uniform float uTanBias = tan(30.0 * PI / 180.0);
uniform float uMaxRadiusPixels = 50.0;

uniform float uNumDirections = 4;
uniform float uNumSteps = 4;


vec3 getViewSpacePos(vec2 uv)
{
	float depth = texture(uDepthMap, uv).r;
	vec4 clipSpacePosition = vec4(vec3(uv, depth) * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;
	viewSpacePosition.z = -viewSpacePosition.z;
	return viewSpacePosition.xyz;
}

float tanToSin(float x)
{
	return x * inversesqrt(x * x + 1.0);
}

float invLength(vec2 v)
{
	return inversesqrt(dot(v, v));
}

float tangent(vec3 T)
{
	return -T.z * invLength(T.xy);
}

float tangent(vec3 P, vec3 S)
{
    return (P.z - S.z) * invLength(S.xy - P.xy);
}

float biasedTangent(vec3 T)
{
	return tangent(T) + uTanBias;
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

float falloff(float d2)
{
	return d2 * uNegInvR2 + 1.0;
}

float horizonOcclusion(	vec2 deltaUV,
						vec3 P,
						vec3 dPdu,
						vec3 dPdv,
						float randstep,
						float numSamples)
{
	float ao = 0;

	// Offset the first coord with some noise
	vec2 uv = vTexCoord + snapUVOffset(randstep * deltaUV);
	deltaUV = snapUVOffset(deltaUV);

	// Calculate the tangent vector
	vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

	// Get the angle of the tangent vector from the viewspace axis
	float tanH = biasedTangent(T);
	float sinH = tanToSin(tanH);

	// Sample to find the maximum angle
	for(float s = 1.0; s <= numSamples; ++s)
	{
		uv += deltaUV;
		vec3 S = getViewSpacePos(uv);
		float tanS = tangent(P, S);
		float d2 = length2(S - P);

		// Is the sample within the radius and the angle greater?
		if(d2 < uRadius2 && tanS > tanH)
		{
			float sinS = tanToSin(tanS);
			// Apply falloff based on the distance
			ao += falloff(d2) * (sinS - sinH);

			tanH = tanS;
			sinH = sinS;
		}
	}
	
	return ao;
}

vec2 rotateDirections(vec2 dir, vec2 cosSin)
{
    return vec2(dir.x * cosSin.x - dir.y * cosSin.y, dir.x * cosSin.y + dir.y * cosSin.x);
}

void computeSteps(inout vec2 stepSizeUv, inout float numSteps, float rayRadiusPix, float rand)
{
    // Avoid oversampling if numSteps is greater than the kernel radius in pixels
    numSteps = min(uNumSteps, rayRadiusPix);

    // Divide by Ns+1 so that the farthest samples are not fully attenuated
    float stepSizePix = rayRadiusPix / (numSteps + 1.0);

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

	vec3 P 	= getViewSpacePos(vTexCoord);

    // Get the random samples from the noise texture
	vec3 random = texture(uNoiseMap, vTexCoord.xy * uNoiseScale).rgb;

	// Calculate the projected size of the hemisphere
    vec2 rayRadiusUV = 0.5 * uRadius * uFocalLength / P.z;
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

		ao = 0.0;
		float alpha = 2.0 * PI / uNumDirections;

		// Calculate the horizon occlusion of each direction
		for(float d = 0; d < uNumDirections; ++d)
		{
			float theta = alpha * d;

			// Apply noise to the direction
			vec2 dir = rotateDirections(vec2(cos(theta), sin(theta)), random.xy);
			vec2 deltaUV = dir * stepSizeUV;

			// Sample the pixels along the direction
			ao += horizonOcclusion(	deltaUV,
									P,
									dPdu,
									dPdv,
									random.z,
									numSteps);
		}

		// Average the results and produce the final AO
		ao = pow(clamp(1.0 - ao / uNumDirections, 0.0, 1.0), uStrength);
	}

	oColor = vec4(ao, P.z, 0.0, 0.0);
}