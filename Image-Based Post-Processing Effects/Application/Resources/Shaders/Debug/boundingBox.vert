#version 450 core

layout (location = 0) in vec3 aPosition;  
  
uniform mat4 uModelViewProjectionMatrix;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0); 
}