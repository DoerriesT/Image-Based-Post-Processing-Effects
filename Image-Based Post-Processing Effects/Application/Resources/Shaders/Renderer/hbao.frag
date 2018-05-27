#version 330 core

out vec4 oFragColor;
in vec2 vTexCoord;

uniform sampler2D uDepthMap;
uniform sampler2D uNormalMap;
uniform sampler2D uNoiseMap; //(cos(alpha), sin(alpha), beta); αE[0,2π/Nd] and β [0,1).
uniform float uRadius; // view space
uniform float uDirections;
uniform float uNumSteps;
uniform float uAngleBias;
uniform float uStrength;
uniform float uMaxRadiusPixels;
uniform vec2 uFocalLength; // (cotan(fovy / 2), h / w, cotan(fovy / 2))
uniform mat4 uInverseProjection;

const float Z_NEAR = 0.1;
const float Z_FAR = 3000.0;
const float PI = 3.14159265;

vec2 gTexSize = vec2(0.0);
vec2 gTexelSize = vec2(0.0);
float gNegInvR2 = 1.0;
float gR2 = 1.0;

vec3 getViewSpacePos(vec2 uv)
{
	float depth = texture(uDepthMap, uv).r;
	vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;
	viewSpacePosition.z = -viewSpacePosition.z;
	return viewSpacePosition.xyz;
}

float invLength(vec2 v)
{
    return inversesqrt(dot(v,v));
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
    // Do not use atan() because it gets expanded by fxc to many math instructions
    return tangent(T) + uAngleBias;
}

vec3 tangentVector(vec2 deltaUV, vec3 dPdu, vec3 dPdv)
{
    return deltaUV.x * dPdu + deltaUV.y * dPdv;
}

float tanToSin(float x)
{
    return x * inversesqrt(x*x + 1.0f);
}

vec2 rotateDirections(vec2 Dir, vec2 CosSin)
{
    return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y, Dir.x*CosSin.y + Dir.y*CosSin.x);
}

float length2(vec3 v)
{
    return dot(v, v);
}

float falloff(float d2)
{
    // 1 scalar mad instruction
    return d2 * gNegInvR2 + 1.0f;
}

vec3 minDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (length2(V1) < length2(V2)) ? V1 : V2;
}

vec2 snapUVOffset(vec2 uv)
{
    return round(uv * gTexSize) * gTexelSize;
}

void computeSteps(inout vec2 stepSizeUV, inout float numSteps, float radiusPix, float rand)
{
    // Avoid oversampling if NUM_STEPS is greater than the kernel radius in pixels
    numSteps = min(uNumSteps, radiusPix);

    // Divide by Ns+1 so that the farthest samples are not fully attenuated
    float stepSizePix = radiusPix / (numSteps + 1.0);

    // Clamp numSteps if it is greater than the max kernel footprint
    float maxNumSteps = uMaxRadiusPixels / stepSizePix;
    if (maxNumSteps < numSteps)
    {
        // Use dithering to avoid AO discontinuities
        numSteps = floor(maxNumSteps + rand);
		numSteps = max(numSteps, 1.0);
        stepSizePix = uMaxRadiusPixels / numSteps;
    }

    // Step size in uv space
    stepSizeUV = stepSizePix * gTexelSize;
}

float integerateOcclusion(vec2 uv0,
                          vec2 snappedDuv,
                          vec3 P,
                          vec3 dPdu,
                          vec3 dPdv,
                          inout float tanH)
{
    float ao = 0;

    // Compute a tangent vector for snapped_duv
    vec3 T1 = tangentVector(snappedDuv, dPdu, dPdv);
    float tanT = biasedTangent(T1);
    float sinT = tanToSin(tanT);

    vec3 S = getViewSpacePos(uv0 + snappedDuv);
    float tanS = tangent(P, S);

    float sinS = tanToSin(tanS);
    float d2 = length2(S - P);

    if ((d2 < gR2) && (tanS > tanT))
    {
        // Compute AO between the tangent plane and the sample
        ao = falloff(d2) * (sinS - sinT);

        // Update the horizon angle
        tanH = max(tanH, tanS);
    }

    return ao;
}

float horizonOcclusion(vec2 deltaUV,
                        vec2 texelDeltaUV,
                        vec2 uv0,
                        vec3 P,
                        float numSteps,
                        float randstep,
                        vec3 dPdu,
                        vec3 dPdv )
{
    float ao = 0;

    // Randomize starting point within the first sample distance
    vec2 uv = uv0 + snapUVOffset( randstep * deltaUV );

    // Snap increments to pixels to avoid disparities between xy
    // and z sample locations and sample along a line
    deltaUV = snapUVOffset( deltaUV );

    // Compute tangent vector using the tangent plane
    vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

    float tanH = biasedTangent(T);

#if SAMPLE_FIRST_STEP
    // Take a first sample between uv0 and uv0 + deltaUV
    //vec2 snappedDuv = snapUVOffset( randstep * deltaUV + texelDeltaUV );
    //ao = integerateOcclusion(uv0, snapped_duv, P, dPdu, dPdv, tanH);
    //--numSteps;
#endif

    float sinH = tanH / sqrt(1.0f + tanH*tanH);

    for (float j = 1; j <= numSteps; ++j)
    {
        uv += deltaUV;
        vec3 S = getViewSpacePos(uv);
        float tanS = tangent(P, S);
        float d2 = length2(S - P);

        // Use a merged dynamic branch
        if ((d2 < gR2) && (tanS > tanH))
        {
            // Accumulate AO between the horizon and the sample
            float sinS = tanS / sqrt(1.0f + tanS*tanS);
            ao += falloff(d2) * (sinS - sinH);

            // Update the current horizon angle
            tanH = tanS;
            sinH = sinS;
        }
    }

    return ao;
}

void main()
{
	// get center view space position
	vec3 P = getViewSpacePos(vTexCoord);
	
	vec3 noiseValues = texture(uNoiseMap, vTexCoord / 4).xyz;
	
	gTexSize = textureSize(uNormalMap, 0);
	gTexelSize = 1.0 / gTexSize;
	gR2 = uRadius * uRadius;
	gNegInvR2 = -(1.0 / gR2);
	
	// project radius into screen space
	// Multiply by 0.5 to scale from [-1,1]^2 to [0,1]^2
	vec2 radiusUV = 0.5 * uRadius * uFocalLength / P.z;
	float radiusPix = radiusUV.x * gTexSize.x;
	
	if (radiusPix >= 1.0)
	{
		
		float numSteps;
		vec2 stepSize;
		computeSteps(stepSize, numSteps, radiusPix, noiseValues.z);
		
		// Nearest neighbor pixels on the tangent plane
		vec3 Pr, Pl, Pt, Pb;
		Pr = getViewSpacePos(vTexCoord + vec2(gTexelSize.x, 0.0));
		Pl = getViewSpacePos(vTexCoord + vec2(-gTexelSize.x, 0.0));
		Pt = getViewSpacePos(vTexCoord + vec2(0.0, gTexelSize.y));
		Pb = getViewSpacePos(vTexCoord + vec2(0.0, -gTexelSize.y));
		
		// Screen-aligned basis for the tangent plane
		vec3 dPdu = minDiff(P, Pr, Pl);
		vec3 dPdv = minDiff(P, Pt, Pb) * (gTexSize.y * gTexelSize.x);
		
		float ao = 0.0;
		float alpha = 2.0 * PI / uDirections;
		
		for (float d = 0; d < uDirections; ++d)
		{
			float angle = alpha * d;
			vec2 dir = rotateDirections(vec2(cos(angle), sin(angle)), noiseValues.xy);
			vec2 deltaUV = dir * stepSize.xy;
			vec2 texelDeltaUV = dir * gTexelSize;
			ao += horizonOcclusion(deltaUV, texelDeltaUV, vTexCoord, P, numSteps, noiseValues.z, dPdu, dPdv);
		}
		
		
		ao = 1.0 - ao / uDirections * uStrength;
		oFragColor = vec4(ao, 0.0, 0.0, 0.0);
		
	}
	else
	{
		oFragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
}