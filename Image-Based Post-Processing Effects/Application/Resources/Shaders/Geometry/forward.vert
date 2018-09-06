#version 450 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec2 aTexCoords; 
layout (location = 2) in vec3 aNormal; 
  
out vec2 vTexCoord;
out vec3 vNormal;
out vec4 vCurrentPos;
out vec4 vPrevPos;
out vec3 vWorldPos;

uniform mat4 uPrevTransform;
uniform mat4 uCurrTransform;
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform vec4 uAtlasData; // x = 1/cols, y = 1/rows, z = texOffsetX, w = texOffsetY


void main()
{
	// Support for texture atlas, update texture coordinates
    vTexCoord = aTexCoords.xy * uAtlasData.xy + uAtlasData.zw;

	vNormal = mat3(uModelMatrix) * aNormal;
	vCurrentPos = uCurrTransform * vec4(aPosition, 1.0);
	vPrevPos = uPrevTransform * vec4(aPosition, 1.0);
	vec4 pos  = uModelMatrix * vec4(aPosition, 1.0);
	vWorldPos = pos.xyz / pos.w;
	
	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0); 
}