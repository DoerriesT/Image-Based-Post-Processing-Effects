#version 450 core

layout (location = 0) in vec3 aPosition;

const int SHADOW_CASCADES = 4;

uniform mat4 uModelViewProjectionMatrix[SHADOW_CASCADES];

out int vInstance;

void main()
{
	vInstance = gl_InstanceID;
	gl_Position = uModelViewProjectionMatrix[gl_InstanceID] * vec4(aPosition, 1.0);
}