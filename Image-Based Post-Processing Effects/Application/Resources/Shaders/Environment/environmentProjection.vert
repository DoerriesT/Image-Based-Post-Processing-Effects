#version 450

out vec3 vNormal;

uniform mat4 uInverseProjection;
uniform mat3 uRotation;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 1.0, 1.0);
	vec4 ray = uInverseProjection * vec4(gl_Position.xy, -1.0,  1.0);
	vNormal = uRotation * (ray/ray.w).xyz;
}
