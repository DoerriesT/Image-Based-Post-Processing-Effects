#version 450 core

out vec3 teWorldPos;

uniform mat4 uInvLightViewProjection;
uniform mat4 uViewProjection;
uniform mat4 uInvViewProjection;
uniform int uPassMode;
uniform vec3 uCamPos;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);

	gl_Position = vec4(x, y, 1, 1);
	
	vec4 worldPos = (uPassMode == 1 ? uInvLightViewProjection : uInvViewProjection) * gl_Position;
	worldPos *= 1.0 / worldPos.w;
	teWorldPos = worldPos.xyz;
}