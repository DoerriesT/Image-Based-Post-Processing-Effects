#version 330 core

layout (location = 0) in vec3 aPosition;

out vec2 vTexCoord;

uniform vec4 uTransform; // x,y = position; z,w = scale

void main()
{

	//calc texture coords based on position
	vTexCoord = aPosition.xz + vec2(0.5, 0.5);
	//apply position and scale to quad
	vec2 screenPosition = aPosition.xz * uTransform.zw + uTransform.xy;
	
	//convert to OpenGL coordinate system (with (0,0) in center of screen)
	//screenPosition.x = screenPosition.x * 2.0 - 1.0;
	//screenPosition.y = screenPosition.y * -2.0 + 1.0;
	gl_Position = vec4(screenPosition, 1.0, 1.0);
}