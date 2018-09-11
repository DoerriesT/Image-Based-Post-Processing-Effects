#version 450 core

flat out ivec2 vCellIndex;

uniform vec3 uGridSize;

void main()
{
	int width = int(uGridSize.x * uGridSize.z);
	ivec2 coord = ivec2(gl_VertexID % width, gl_VertexID / width);

	vCellIndex = coord;

	vec2 outputCoord = vec2(coord) + vec2(0.5);
	outputCoord /= vec2(uGridSize.x * uGridSize.z, uGridSize.y);

	gl_Position = vec4(outputCoord * 2.0 - 1.0, 0.0, 1.0);
}