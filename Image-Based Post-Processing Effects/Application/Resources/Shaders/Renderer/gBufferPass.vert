#version 450 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec2 aTexCoords; 
layout (location = 2) in vec3 aNormal; 
layout (location = 3) in vec3 aTangent; 
  
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec4 vCurrentPos;
out vec4 vPrevPos;

uniform mat4 uPrevTransform;
uniform mat4 uCurrTransform;
uniform mat4 uModelViewProjectionMatrix;
uniform mat3 uModelViewMatrix;
uniform vec4 uAtlasData; // x = 1/cols, y = 1/rows, z = texOffsetX, w = texOffsetY


void main()
{
	// Support for texture atlas, update texture coordinates
    vTexCoord = aTexCoords.xy * uAtlasData.xy + uAtlasData.zw;

    vNormal = uModelViewMatrix * aNormal;
	vTangent = uModelViewMatrix * aTangent;
	vBitangent = uModelViewMatrix * cross(vNormal, vTangent);

	vPrevPos = uPrevTransform * vec4(aPosition, 1.0);
	vCurrentPos = uCurrTransform * vec4(aPosition, 1.0);
	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0);
}