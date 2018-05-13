#version 330

out vec4 vRay;
out vec4 vCurrentPos;
out vec4 vPrevPos;

uniform mat4 uTransform;
uniform mat4 uPrevTransform;
uniform mat4 uInverseModelViewProjection;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 1.0, 1.0);
	vRay = uInverseModelViewProjection * vec4(gl_Position.xy, -1.0,  1.0);
	vCurrentPos = uTransform * vec4(gl_Position.xy, -1.0,  1.0);
	vPrevPos = uPrevTransform * vec4(gl_Position.xy, -1.0,  1.0);
}
