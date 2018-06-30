#version 330

#define FXAA_QUALITY__PS 5
#define FXAA_QUALITY__P0 1.0
#define FXAA_QUALITY__P1 1.5
#define FXAA_QUALITY__P2 2.0
#define FXAA_QUALITY__P3 4.0
#define FXAA_QUALITY__P4 12.0

out vec4 oColor;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uScreenTexture;

uniform vec2 uInverseResolution; // inverse resolution
// Only used on FXAA Quality.
    // This used to be the FXAA_QUALITY__SUBPIX define.
    // It is here now to allow easier tuning.
    // Choose the amount of sub-pixel aliasing removal.
    // This can effect sharpness.
    //   1.00 - upper limit (softer)
    //   0.75 - default amount of filtering
    //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
    //   0.25 - almost off
    //   0.00 - completely off
uniform float uSubPixelAA;
// Only used on FXAA Quality.
    // This used to be the FXAA_QUALITY__EDGE_THRESHOLD define.
    // It is here now to allow easier tuning.
    // The minimum amount of local contrast required to apply algorithm.
    //   0.333 - too little (faster)
    //   0.250 - low quality
    //   0.166 - default
    //   0.125 - high quality 
    //   0.063 - overkill (slower)
uniform float uEdgeThreshold;
// Only used on FXAA Quality.
    // This used to be the FXAA_QUALITY__EDGE_THRESHOLD_MIN define.
    // It is here now to allow easier tuning.
    // Trims the algorithm from processing darks.
    //   0.0833 - upper limit (default, the start of visible unfiltered edges)
    //   0.0625 - high quality (faster)
    //   0.0312 - visible limit (slower)
    // Special notes when using FXAA_GREEN_AS_LUMA,
    //   Likely want to set this to zero.
    //   As colors that are mostly not-green
    //   will appear very dark in the green channel!
    //   Tune by looking at mostly non-green content,
    //   then start at zero and increase until aliasing is a problem.
uniform float uEdgeThresholdMin;


bool fxaa(inout vec3 color)
{
	vec2 posM;
    posM.x = vTexCoord.x;
    posM.y = vTexCoord.y;
    vec4 rgbyM = textureLod(uScreenTexture, posM, 0.0);

    #define lumaM rgbyM.w

    float lumaS = textureLodOffset(uScreenTexture, posM, 0.0, ivec2( 0, 1)).a;
    float lumaE = textureLodOffset(uScreenTexture, posM, 0.0, ivec2( 1, 0)).a;
    float lumaN = textureLodOffset(uScreenTexture, posM, 0.0, ivec2( 0,-1)).a;
    float lumaW = textureLodOffset(uScreenTexture, posM, 0.0, ivec2(-1, 0)).a;


    float maxSM = max(lumaS, lumaM);
    float minSM = min(lumaS, lumaM);
    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);
    float maxWN = max(lumaN, lumaW);
    float minWN = min(lumaN, lumaW);
    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);
    float rangeMaxScaled = rangeMax * uEdgeThreshold;
    float range = rangeMax - rangeMin;
    float rangeMaxClamped = max(uEdgeThresholdMin, rangeMaxScaled);
    bool earlyExit = range < rangeMaxClamped;

    if(earlyExit)
    {
		return false;
	}


    float lumaNW = textureLodOffset(uScreenTexture, posM, 0.0, ivec2(-1,-1)).a;
    float lumaSE = textureLodOffset(uScreenTexture, posM, 0.0, ivec2( 1, 1)).a;
    float lumaNE = textureLodOffset(uScreenTexture, posM, 0.0, ivec2( 1,-1)).a;
    float lumaSW = textureLodOffset(uScreenTexture, posM, 0.0, ivec2(-1, 1)).a;
   

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float subpixRcpRange = 1.0/range;
    float subpixNSWE = lumaNS + lumaWE;
    float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    float edgeVert1 = (-2.0 * lumaM) + lumaWE;

    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
    float edgeHorz = abs(edgeHorz3) + edgeHorz4;
    float edgeVert = abs(edgeVert3) + edgeVert4;

    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float lengthSign = uInverseResolution.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;

    if(!horzSpan) 
	{
		lumaN = lumaW;
	}
    if(!horzSpan) 
	{
		lumaS = lumaE;
	}
    if(horzSpan) 
	{
		lengthSign = uInverseResolution.y;
	}
    float subpixB = (subpixA * (1.0/12.0)) - lumaM;

    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    bool pairN = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if(pairN)
	{
		lengthSign = -lengthSign;
	}
    float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);

    vec2 posB;
    posB.x = posM.x;
    posB.y = posM.y;
    vec2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : uInverseResolution.x;
    offNP.y = ( horzSpan) ? 0.0 : uInverseResolution.y;
    if(!horzSpan)
	{
		posB.x += lengthSign * 0.5;
	}
    if( horzSpan) 
	{
		posB.y += lengthSign * 0.5;
	}

    vec2 posN;
    posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;
    posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;
    vec2 posP;
    posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;
    posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;
    float subpixD = ((-2.0)*subpixC) + 3.0;
    float lumaEndN = textureLod(uScreenTexture, posN, 0.0).a;
    float subpixE = subpixC * subpixC;
    float lumaEndP = textureLod(uScreenTexture, posP, 0.0).a;

    if(!pairN) 
	{
		lumaNN = lumaSS;
	}
    float gradientScaled = gradient * 1.0/4.0;
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = subpixD * subpixE;
    bool lumaMLTZero = lumaMM < 0.0;

    lumaEndN -= lumaNN * 0.5;
    lumaEndP -= lumaNN * 0.5;
    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN)
	{
		posN.x -= offNP.x * FXAA_QUALITY__P1;
	}
    if(!doneN) 
	{
		posN.y -= offNP.y * FXAA_QUALITY__P1;
	}
    bool doneNP = (!doneN) || (!doneP);
    if(!doneP)
	{
		posP.x += offNP.x * FXAA_QUALITY__P1;
	}
    if(!doneP) 
	{
		posP.y += offNP.y * FXAA_QUALITY__P1;
	}

    if(doneNP) 
	{
        if(!doneN) 
		{
			lumaEndN = textureLod(uScreenTexture, posN.xy, 0.0).a;
		}
        if(!doneP) 
		{
			lumaEndP = textureLod(uScreenTexture, posP.xy, 0.0).a;
		}
        if(!doneN) 
		{
			lumaEndN = lumaEndN - lumaNN * 0.5;
		}
        if(!doneP) 
		{
			lumaEndP = lumaEndP - lumaNN * 0.5;
		}
        doneN = abs(lumaEndN) >= gradientScaled;
        doneP = abs(lumaEndP) >= gradientScaled;
        if(!doneN)
		{
			posN.x -= offNP.x * FXAA_QUALITY__P2;
		}
        if(!doneN) 
		{
			posN.y -= offNP.y * FXAA_QUALITY__P2;
		}
        doneNP = (!doneN) || (!doneP);
        if(!doneP)
		{
			posP.x += offNP.x * FXAA_QUALITY__P2;
		}
        if(!doneP)
		{
			posP.y += offNP.y * FXAA_QUALITY__P2;
		}

        if(doneNP)
		{
            if(!doneN) 
			{
				lumaEndN = textureLod(uScreenTexture, posN.xy, 0.0).a;
			}
			if(!doneP) 
			{
				lumaEndP = textureLod(uScreenTexture, posP.xy, 0.0).a;
			}
			if(!doneN) 
			{
				lumaEndN = lumaEndN - lumaNN * 0.5;
			}
			if(!doneP) 
			{
				lumaEndP = lumaEndP - lumaNN * 0.5;
			}
			doneN = abs(lumaEndN) >= gradientScaled;
			doneP = abs(lumaEndP) >= gradientScaled;
			if(!doneN)
			{
				posN.x -= offNP.x * FXAA_QUALITY__P3;
			}
			if(!doneN) 
			{
				posN.y -= offNP.y * FXAA_QUALITY__P3;
			}
			doneNP = (!doneN) || (!doneP);
			if(!doneP)
			{
				posP.x += offNP.x * FXAA_QUALITY__P3;
			}
			if(!doneP)
			{
				posP.y += offNP.y * FXAA_QUALITY__P3;
			}

            if(doneNP)
			{
				if(!doneN) 
				{
					lumaEndN = textureLod(uScreenTexture, posN.xy, 0.0).a;
				}
				if(!doneP) 
				{
					lumaEndP = textureLod(uScreenTexture, posP.xy, 0.0).a;
				}
				if(!doneN) 
				{
					lumaEndN = lumaEndN - lumaNN * 0.5;
				}
				if(!doneP) 
				{
					lumaEndP = lumaEndP - lumaNN * 0.5;
				}
				doneN = abs(lumaEndN) >= gradientScaled;
				doneP = abs(lumaEndP) >= gradientScaled;
				if(!doneN)
				{
					posN.x -= offNP.x * FXAA_QUALITY__P4;
				}
				if(!doneN) 
				{
					posN.y -= offNP.y * FXAA_QUALITY__P4;
				}
				doneNP = (!doneN) || (!doneP);
				if(!doneP)
				{
					posP.x += offNP.x * FXAA_QUALITY__P4;
				}
				if(!doneP)
				{
					posP.y += offNP.y * FXAA_QUALITY__P4;
				}
			}
		}
    }

    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if(!horzSpan) dstN = posM.y - posN.y;
    if(!horzSpan) dstP = posP.y - posM.y;

    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    float spanLength = (dstP + dstN);
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    float spanLengthRcp = 1.0/spanLength;

    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    float subpixG = subpixF * subpixF;
    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    float subpixH = subpixG * uSubPixelAA;

    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
    if(!horzSpan) 
	{
		posM.x += pixelOffsetSubpix * lengthSign;
	}
    if( horzSpan) 
	{
		posM.y += pixelOffsetSubpix * lengthSign;
	}
	color = textureLod(uScreenTexture, posM, 0.0).rgb;
    return true;
}

void main()
{
	vec3 color = vec3(0.0);
	if(!fxaa(color))
	{
		color = texture(uScreenTexture, vTexCoord).rgb;
	}
	oColor = vec4(color, 1.0);
}