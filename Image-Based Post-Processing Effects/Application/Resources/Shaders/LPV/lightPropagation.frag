#version 450 core

layout(location = 0) out vec4 oRed;
layout(location = 1) out vec4 oGreen;
layout(location = 2) out vec4 oBlue;

layout(binding = 2) uniform sampler2D uRedTexture;
layout(binding = 3) uniform sampler2D uGreenTexture;
layout(binding = 4) uniform sampler2D uBlueTexture;

uniform vec3 uGridSize;

/*Spherical harmonics coefficients – precomputed*/
#define SH_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_C1 0.488602512f // sqrt(3/pi) / 2

/*Cosine lobe coeff*/
#define SH_cosLobe_C0 0.886226925f // sqrt(pi)/2
#define SH_cosLobe_C1 1.02332671f // sqrt(pi/3)
#define PI 3.1415926f

vec4 dirToCosineLobe(vec3 dir) 
{
	return vec4(SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x);
}

vec4 dirToSh(vec3 dir) 
{
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}

vec3 directions[6] =
{ 
	vec3(0,0,1), vec3(0,0,-1), vec3(1,0,0), vec3(-1,0,0) , vec3(0,1,0), vec3(0,-1,0)
};

// right up
vec2 side[4] = 
{ 
	vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(-1.0, 0.0), vec2(0.0, -1.0) 
};

// orientation = [ right | up | forward ] = [ x | y | z ]
vec3 getEvalSideDirection(uint index, mat3 orientation) 
{
	const float smallComponent = 0.4472135; // 1 / sqrt(5)
	const float bigComponent = 0.894427; // 2 / sqrt(5)
	
	const vec2 s = side[index];
	// *either* x = 0 or y = 0
	return orientation * vec3(s.x * smallComponent, s.y * smallComponent, bigComponent);
}

vec3 getReprojSideDirection(uint index, mat3 orientation) 
{
	const vec2 s = side[index];
	return orientation * vec3(s.x, s.y, 0);
}

// orientation = [ right | up | forward ] = [ x | y | z ]
mat3 neighborOrientations[6] = 
{
	// Z+
	transpose(mat3(1, 0, 0,0, 1, 0,0, 0, 1)),
	// Z-
	transpose(mat3(-1, 0, 0,0, 1, 0,0, 0, -1)),
	// X+
	transpose(mat3(0, 0, 1,0, 1, 0,-1, 0, 0)),
	// X-
	transpose(mat3(0, 0, -1,0, 1, 0,1, 0, 0)),
	// Y+
	transpose(mat3(1, 0, 0,0, 0, 1,0, -1, 0)),
	// Y-
	transpose(mat3(1, 0, 0,0, 0, -1,0, 1, 0))
};

void main()
{
	vec4 coeffsRed = vec4(0.0);
	vec4 coeffsGreen = vec4(0.0);
	vec4 coeffsBlue = vec4(0.0);
	
	ivec3 cellIndex = ivec3(int(gl_FragCoord.x) % int(uGridSize.x), gl_FragCoord.y, int(gl_FragCoord.x) / int(uGridSize.x));
	
	for (int neighbor = 0; neighbor < 6; ++neighbor)
	{
		mat3 orientation = neighborOrientations[neighbor];
		vec3 mainDirection = orientation * vec3(0.0, 0.0, 1.0);
		
		ivec3 neighborIndex = cellIndex - ivec3(directions[neighbor]);
		ivec2 neighborCoord = ivec2(uGridSize.x * neighborIndex.z + neighborIndex.x, neighborIndex.y);
		vec4 coeffsNeighborRed = texelFetch(uRedTexture, neighborCoord, 0).xyzw;
		vec4 coeffsNeighborGreen = texelFetch(uGreenTexture, neighborCoord, 0).xyzw;
		vec4 coeffsNeighborBlue = texelFetch(uBlueTexture, neighborCoord, 0).xyzw;
		
		const float directFaceSubtendedSolidAngle = 0.4006696846 / PI / 2;
		const float sideFaceSubtendedSolidAngle = 0.4234413544 / PI / 3;
		
		for (int sideFace = 0; sideFace < 4; ++sideFace)
		{
			vec3 evalDirection = getEvalSideDirection(sideFace, orientation);
			vec3 reprojDirection = getReprojSideDirection(sideFace, orientation);
			
			vec4 reprojDirectionCosineLobeSh = dirToCosineLobe(reprojDirection);
			vec4 evalDirectionSh = dirToSh(evalDirection);
			
			coeffsRed += directFaceSubtendedSolidAngle * max(0.0, dot(coeffsNeighborRed, evalDirectionSh)) * reprojDirectionCosineLobeSh;
			coeffsGreen += directFaceSubtendedSolidAngle * max(0.0, dot(coeffsNeighborGreen, evalDirectionSh)) * reprojDirectionCosineLobeSh;
			coeffsBlue += directFaceSubtendedSolidAngle * max(0.0, dot(coeffsNeighborBlue, evalDirectionSh)) * reprojDirectionCosineLobeSh;
		}
		
		vec3 curDir = directions[neighbor];
		vec4 curCosLobe = dirToCosineLobe(curDir);
		vec4 curDirSh = dirToSh(curDir);
		
		coeffsRed += directFaceSubtendedSolidAngle * max(0.0, dot(coeffsNeighborRed, curDirSh)) * curCosLobe;
		coeffsGreen += directFaceSubtendedSolidAngle * max(0.0, dot(coeffsNeighborGreen, curDirSh)) * curCosLobe;
		coeffsBlue += directFaceSubtendedSolidAngle * max(0.0, dot(coeffsNeighborBlue, curDirSh)) * curCosLobe;
	}
	
	vec4 prevRed = texelFetch(uRedTexture, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 prevGreen = texelFetch(uGreenTexture, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 prevBlue = texelFetch(uBlueTexture, ivec2(gl_FragCoord.xy), 0).xyzw;
	
	oRed = prevRed + coeffsRed;
	oGreen = prevGreen + coeffsGreen;
	oBlue = prevBlue + coeffsBlue;
}