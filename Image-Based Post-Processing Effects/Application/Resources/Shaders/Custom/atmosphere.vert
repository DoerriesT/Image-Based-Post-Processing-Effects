#version 330 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords; 
layout (location = 3) in vec3 aTangent; 
layout (location = 4) in vec3 aBitangent; 

out vec4 vWorldPos;
out vec2 vTexCoord;
out vec3 vNormal;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;


void main()
{
	vec4 position = vec4(aPosition, 1.0);
    vWorldPos = uModelMatrix * position;
	vNormal = (uModelMatrix * vec4(aNormal, 0.0)).xyz;
	
	gl_Position = uModelViewProjectionMatrix * position;   
}