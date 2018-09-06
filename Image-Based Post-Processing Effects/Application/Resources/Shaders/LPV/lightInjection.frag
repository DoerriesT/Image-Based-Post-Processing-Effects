#version 450 core

layout(location = 0) out vec4 oRed;
layout(location = 1) out vec4 oGreen;
layout(location = 2) out vec4 oBlue;

in vec3 vFlux;
in vec3 vNormal;
in vec3 vGridCell;

layout(binding = 2) uniform sampler2D uRedTexture;
layout(binding = 3) uniform sampler2D uGreenTexture;
layout(binding = 4) uniform sampler2D uBlueTexture;

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
	vec4 prevRed = texelFetch(uRedTexture, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 prevGreen = texelFetch(uGreenTexture, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 prevBlue = texelFetch(uBlueTexture, ivec2(gl_FragCoord.xy), 0).xyzw;
	
	vec4 coeffsRed = evalCosineLobeToDir(vNormal)/ PI * vFlux.r;
	vec4 coeffsGreen = evalCosineLobeToDir(vNormal)/ PI * vFlux.g;
	vec4 coeffsBlue = evalCosineLobeToDir(vNormal)/ PI * vFlux.b;
	
	// todo: weight contribution by surfel size
	oRed = prevRed + coeffsRed;
	oGreen = prevGreen + coeffsGreen;
	oBlue = prevBlue + coeffsBlue;
}