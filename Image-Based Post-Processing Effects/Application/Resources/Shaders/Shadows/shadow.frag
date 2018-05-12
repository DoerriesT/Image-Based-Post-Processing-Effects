#version 330 core

out vec4 oFragColor;  

in vec4 vPosition;

void main()
{
	float depth = vPosition.z / vPosition.w ;
	depth = depth * 0.5 + 0.5;

	float moment1 = depth;
	float moment2 = depth * depth;
	
	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25 * (dx * dx + dy * dy) ;
		
	gl_FragColor = vec4(moment1, moment2, 0.0, 1.0 );
}