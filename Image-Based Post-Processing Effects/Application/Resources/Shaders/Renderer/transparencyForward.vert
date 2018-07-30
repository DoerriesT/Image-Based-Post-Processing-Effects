#version 450 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec2 aTexCoords; 
layout (location = 2) in vec3 aNormal; 
layout (location = 3) in vec3 aTangent; 
  
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec4 vWorldPos;
out vec4 vCurrentPos;
out vec4 vPrevPos;

uniform mat4 uPrevTransform;
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform vec4 uAtlasData; // x = 1/cols, y = 1/rows, z = texOffsetX, w = texOffsetY


void main()
{
	// Support for texture atlas, update texture coordinates
    vTexCoord = aTexCoords.xy * uAtlasData.xy + uAtlasData.zw;

    vNormal = (uModelMatrix * vec4(aNormal, 0.0)).xyz;
	vTangent = (uModelMatrix * vec4(aTangent, 0.0)).xyz;
	vBitangent = (uModelMatrix * vec4(cross(vNormal, vTangent), 0.0)).xyz;

	vec4 position = vec4(aPosition, 1.0);
    vWorldPos = uModelMatrix * position;

	vPrevPos = uPrevTransform * position;
	vCurrentPos = uModelViewProjectionMatrix * position;
	gl_Position = vCurrentPos;   
}