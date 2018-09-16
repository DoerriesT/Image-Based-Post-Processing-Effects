#version 450 core

layout(location = 0) out vec4 oRed;
layout(location = 1) out vec4 oGreen;
layout(location = 2) out vec4 oBlue;

layout(binding = 0) uniform sampler2D uRedTexture;
layout(binding = 1) uniform sampler2D uGreenTexture;
layout(binding = 2) uniform sampler2D uBlueTexture;
layout(binding = 3) uniform sampler2D uGeometryTexture;

layout(rgba32f, binding = 0) uniform image3D uRedAccum;
layout(rgba32f, binding = 1) uniform image3D uGreenAccum;
layout(rgba32f, binding = 2) uniform image3D uBlueAccum;
layout(rgba32f, binding = 3) uniform image2D uAccumRed;
layout(rgba32f, binding = 4) uniform image2D uAccumGreen;
layout(rgba32f, binding = 5) uniform image2D uAccumBlue;

uniform vec3 uGridSize;
uniform bool uFirstIteration;
uniform float uOcclusionAmplifier;

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

vec4 dirToSH(vec3 dir) 
{
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}

//Sides of the cell - right, top, left, bottom
ivec2 cellSides[4] = { ivec2( 1.0, 0.0 ), ivec2( 0.0, 1.0 ), ivec2( -1.0, 0.0 ), ivec2( 0.0, -1.0 ) };

//Get side direction
vec3 getEvalSideDirection( int index, ivec3 orientation ) 
{
	const float smallComponent = 0.4472135; // 1 / sqrt(5)
	const float bigComponent = 0.894427; // 2 / sqrt(5)
	
	const ivec2 side = cellSides[ index ];
	vec3 tmp = vec3( side.x * smallComponent, side.y * smallComponent, bigComponent );
	return vec3(orientation.x * tmp.x, orientation.y * tmp.y, orientation.z * tmp.z);
}

vec3 getReprojSideDirection( int index, ivec3 orientation ) 
{
	const ivec2 side = cellSides[ index ];
	return vec3( orientation.x*side.x, orientation.y*side.y, 0 );
}

const ivec3 directions[6] = 
{
	//+Z
	ivec3(0,0,1),
	//-Z
	ivec3(0,0,-1),
	//+X
	ivec3(1,0,0),
	//-X
	ivec3(-1,0,0),
	//+Y
	ivec3(0,1,0),
	//-Y
	ivec3(0,-1,0)
};

void main()
{
	vec4 coeffsRed = vec4(0.0);
	vec4 coeffsGreen = vec4(0.0);
	vec4 coeffsBlue = vec4(0.0);
	
	ivec3 cellIndex = ivec3(int(gl_FragCoord.x) % int(uGridSize.x), gl_FragCoord.y, int(gl_FragCoord.x) / int(uGridSize.x));
	
	for (int neighbor = 0; neighbor < 6; ++neighbor)
	{
		ivec3 direction = directions[neighbor];
		
		ivec3 neighborIndex = cellIndex - direction;
		ivec2 neighborCoord = ivec2(uGridSize.x * neighborIndex.z + neighborIndex.x, neighborIndex.y);
		vec4 coeffsNeighborRed = texelFetch(uRedTexture, neighborCoord, 0).xyzw;
		vec4 coeffsNeighborGreen = texelFetch(uGreenTexture, neighborCoord, 0).xyzw;
		vec4 coeffsNeighborBlue = texelFetch(uBlueTexture, neighborCoord, 0).xyzw;
		
		const float directFaceSubtendedSolidAngle = 0.4006696846 / PI;
		const float sideFaceSubtendedSolidAngle = 0.4234413544 / PI;

		float occlusionValue = 1.0;

		//No occlusion for the first step
		if(!uFirstIteration) 
		{
			vec3 h_direction = 0.5 * direction;
            ivec2 offset = ivec2(
                h_direction.x + (h_direction.z * float(uGridSize.x)),
                h_direction.y
            );
            ivec2 occCoord = ivec2(gl_FragCoord.xy) - offset;
			
			vec4 occCoeffs = texture(uGeometryTexture, occCoord);
			occlusionValue = 1.0 - clamp( uOcclusionAmplifier * dot(occCoeffs, dirToSH( -vec3(direction))), 0.0, 1.0 );
		}

		vec4 curCosLobe = dirToCosineLobe(direction);
		vec4 curDirSh = dirToSH(direction);
		
		coeffsRed += directFaceSubtendedSolidAngle * occlusionValue * max(0.0, dot(coeffsNeighborRed, curDirSh)) * curCosLobe;
		coeffsGreen += directFaceSubtendedSolidAngle * occlusionValue * max(0.0, dot(coeffsNeighborGreen, curDirSh)) * curCosLobe;
		coeffsBlue += directFaceSubtendedSolidAngle * occlusionValue * max(0.0, dot(coeffsNeighborBlue, curDirSh)) * curCosLobe;
		
		for (int sideFace = 0; sideFace < 4; ++sideFace)
		{
			vec3 evalDirection = getEvalSideDirection(sideFace, directions[neighbor]);
			vec3 reprojDirection = getReprojSideDirection(sideFace, directions[neighbor]);
			
			vec4 reprojDirectionCosineLobeSh = dirToCosineLobe(reprojDirection);
			vec4 evalDirectionSh = dirToSH(evalDirection);
			
			coeffsRed += sideFaceSubtendedSolidAngle * occlusionValue * max(0.0, dot(coeffsNeighborRed, evalDirectionSh)) * reprojDirectionCosineLobeSh;
			coeffsGreen += sideFaceSubtendedSolidAngle * occlusionValue * max(0.0, dot(coeffsNeighborGreen, evalDirectionSh)) * reprojDirectionCosineLobeSh;
			coeffsBlue += sideFaceSubtendedSolidAngle * occlusionValue * max(0.0, dot(coeffsNeighborBlue, evalDirectionSh)) * reprojDirectionCosineLobeSh;
		}
	}
	
	oRed = coeffsRed;
	oGreen = coeffsGreen;
	oBlue = coeffsBlue;

	imageStore(uAccumRed, ivec2(gl_FragCoord.xy), imageLoad(uAccumRed, ivec2(gl_FragCoord.xy)) + coeffsRed);
	imageStore(uAccumGreen, ivec2(gl_FragCoord.xy), imageLoad(uAccumGreen, ivec2(gl_FragCoord.xy)) + coeffsGreen);
	imageStore(uAccumBlue, ivec2(gl_FragCoord.xy), imageLoad(uAccumBlue, ivec2(gl_FragCoord.xy)) + coeffsBlue);
}