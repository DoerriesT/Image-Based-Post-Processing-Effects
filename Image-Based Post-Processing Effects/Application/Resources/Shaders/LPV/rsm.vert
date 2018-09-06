#version 450 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec2 aTexCoords; 
layout (location = 2) in vec3 aNormal; 
  
out vec2 vTexCoord;
out vec3 vNormal;

uniform mat4 uModelViewProjectionMatrix;
uniform mat3 uModelMatrix;
uniform vec4 uAtlasData; // x = 1/cols, y = 1/rows, z = texOffsetX, w = texOffsetY


void main()
{
	// Support for texture atlas, update texture coordinates
    vTexCoord = aTexCoords.xy * uAtlasData.xy + uAtlasData.zw;

	vNormal = uModelMatrix * aNormal;
	
	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0);
}