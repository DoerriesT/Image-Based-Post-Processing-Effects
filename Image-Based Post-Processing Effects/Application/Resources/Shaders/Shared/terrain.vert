#version 450 core

layout (location = 0) in vec2 aPosition;  
layout (location = 1) in vec4 aNeighborData; 

// aNeighborData:
// float neighbourMinusX;
// float neighbourMinusY;
// float neighbourPlusX;
// float neighbourPlusY;

out vec3 vPosition;
out vec4 vNeighborData;

const float RECIP_CONTROL_VTX_PER_TILE_EDGE = 1.0 / 9;
const int CONTROL_VTX_PER_TILE_EDGE = 9;

uniform float uTileSize = 1.0;
uniform mat4 uViewProjection;
uniform vec3 uCamPos;
uniform sampler2D uDisplacementMap;
uniform float uVerticalDisplacement;

void main()
{
    float iv = floor(gl_VertexID * RECIP_CONTROL_VTX_PER_TILE_EDGE);
    float iu = gl_VertexID - iv * CONTROL_VTX_PER_TILE_EDGE;
    float u = iu / (CONTROL_VTX_PER_TILE_EDGE - 1.0);
    float v = iv / (CONTROL_VTX_PER_TILE_EDGE - 1.0);

	float size = uTileSize;

	vec4 pos = vec4(u * size + aPosition.x, 0, v * size + aPosition.y, 1.0);
	pos.xz += round(uCamPos.xz);
	pos.y += uVerticalDisplacement;
	
	// TODO: sample displacement to improve tesselation
	/*
	float z = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, output.vPosition.xz);
	output.vPosition.y += z;
	output.vWorldXZ = output.vPosition.xz;
	output.adjacency = input.adjacency;
	*/
	
	//pos = uViewProjection * pos;
	gl_Position = pos;
	
	vPosition = pos.xyz / pos.w;
	vNeighborData = aNeighborData;
}
