#version 450 core

layout(location = 0) out vec4 oRed;
layout(location = 1) out vec4 oGreen;
layout(location = 2) out vec4 oBlue;

in vec3 vFlux;
in vec3 vNormal;
in vec3 vGridCell;

/*Cosine lobe coeff*/
#define SH_cosLobe_C0 0.886226925f // sqrt(pi)/2 
#define SH_cosLobe_C1 1.02332671f // sqrt(pi/3) 

#define PI 3.1415926f

vec4 evalCosineLobeToDir(vec3 dir) 
{
	//f00, f-11, f01, f11
	return vec4(SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x );
}

void main()
{
	vec4 coeffs = evalCosineLobeToDir(vNormal) / PI;
	
	oRed = coeffs * vFlux.r;
	oGreen = coeffs * vFlux.g;
	oBlue = coeffs * vFlux.b;
}