#version 450 core

out vec3 teWorldPos;

uniform mat4 uInvLightViewProjection;
uniform mat4 uViewProjection;
uniform mat4 uInvViewProjection;
uniform int uPassMode;
uniform vec3 uCamPos;

void main()
{
	if (uPassMode == 0)
	{
		int vtx_idx = gl_VertexID % 3;
		gl_Position.x = (vtx_idx == 0) ? 1 : -1;
		gl_Position.y = (vtx_idx == 2) ? -1 : 1;
		gl_Position.xy *= (gl_VertexID/3 == 0) ? 1 : -1;
		gl_Position.z = 1.0;
		gl_Position.w = 1.0;
	}
	else
	{
		//vec2 vTex = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
		//gl_Position = vec4(vTex * vec2(2,-2) + vec2(-1,1), 1, 1);
		
		float x = -1.0 + float((gl_VertexID & 1) << 2);
		float y = -1.0 + float((gl_VertexID & 2) << 1);
	
		gl_Position = vec4(x, y, 1, 1);
	}
	
	//float x = -1.0 + float((gl_VertexID & 1) << 2);
	//float y = -1.0 + float((gl_VertexID & 2) << 1);

	//gl_Position = vec4(x, y, 1, 1);
	
	vec4 worldPos = (uPassMode == 1 ? uInvLightViewProjection : uInvViewProjection) * gl_Position;
	worldPos *= 1.0 / worldPos.w;
	teWorldPos = worldPos.xyz;
}