#version 450 core

layout (location = 0) in vec3 aPosition;  
  
out vec3 vNormal;

uniform mat4 uModelViewProjectionMatrix;


void main()
{
	vNormal = aPosition;
	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0); 
}