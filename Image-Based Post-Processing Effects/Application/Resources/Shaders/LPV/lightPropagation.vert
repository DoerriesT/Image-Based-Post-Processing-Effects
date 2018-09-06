#version 450 core

uniform vec3 uGridSize;

void main()
{
	int width = int(uGridSize.x * uGridSize.z);
	ivec2 coord = ivec2(gl_VertexID % width, gl_VertexID / width);
	gl_Position = vec4(vec2(coord) * 2.0 - 1.0, 0.0, 1.0);
}