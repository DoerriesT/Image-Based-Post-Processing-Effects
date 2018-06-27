#version 450 core

layout (location = 0) in vec3 aPosition;

const int SHADOW_CASCADES = 4;
const int MAX_MATRICES = 6; // we use this for pointlights -> cubemaps

uniform mat4 uModelViewProjectionMatrix[MAX_MATRICES];

out int vInstance;

void main()
{
	vInstance = gl_InstanceID;
	gl_Position = uModelViewProjectionMatrix[gl_InstanceID] * vec4(aPosition, 1.0);
}