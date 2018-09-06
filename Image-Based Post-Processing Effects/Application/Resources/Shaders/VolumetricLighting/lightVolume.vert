#version 450 core

layout (location = 0) in vec2 aPosition;  

out vec3 vPosition;

const float RECIP_CONTROL_VTX_PER_TILE_EDGE = 1.0 / 65;
const int CONTROL_VTX_PER_TILE_EDGE = 65;


void main()
{
	float iv = floor(gl_VertexID * RECIP_CONTROL_VTX_PER_TILE_EDGE);
    float iu = gl_VertexID - iv * CONTROL_VTX_PER_TILE_EDGE;
    float u = iu / (CONTROL_VTX_PER_TILE_EDGE - 1.0);
    float v = iv / (CONTROL_VTX_PER_TILE_EDGE - 1.0);
		
	vec4 pos = vec4(u, v, 0.0, 1.0);
	
	gl_Position = pos;
	
	vPosition = pos.xyz / pos.w;
}
