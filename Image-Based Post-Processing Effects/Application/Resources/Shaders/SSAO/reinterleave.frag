#version 450 core

layout(location = 0) out vec4 oResult;

layout(binding = 0) uniform sampler2DArray uAoArrayTexture;

void main()
{
	ivec2 fullResPos = ivec2(gl_FragCoord.xy);
	ivec2 offset = fullResPos & 3;
	int sliceId = offset.y * 4 + offset.x;
	ivec2 quarterResPos = fullResPos >> 2;
	
	oResult = vec4(texelFetch(uAoArrayTexture, ivec3(quarterResPos, sliceId), 0).xy, 0.0, 0.0);
}