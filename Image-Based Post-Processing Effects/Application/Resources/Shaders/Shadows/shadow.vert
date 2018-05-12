#version 330 core

layout (location = 0) in vec3 aPosition;  
  
out vec4 vPosition;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;

void main()
{
	vec4 position = vec4(aPosition, 1.0);
    //vWorldPos = vec3(uModelMatrix * position);

	gl_Position = uModelViewProjectionMatrix * position;
	vPosition = gl_Position;
}