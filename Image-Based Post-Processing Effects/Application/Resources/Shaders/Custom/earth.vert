#version 330 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords; 
layout (location = 3) in vec3 aTangent; 
layout (location = 4) in vec3 aBitangent; 
  
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;


void main()
{
    vTexCoord = aTexCoords;

	mat3 modelMatrix = mat3(uModelMatrix);

    vNormal = modelMatrix * aNormal;
	vTangent = modelMatrix * aTangent;
	vBitangent = modelMatrix * aBitangent;

	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0);   
}