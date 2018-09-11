#version 450 core

layout(location = 0) out vec4 oGeometry;

uniform vec3 uLightDirection;
uniform vec2 uGridSpacing; // spacing, 1.0 / spacing

struct RSMTexel
{
	vec3 normal;
	vec3 position;
};

in RSMTexel vRSMTexel;
in float vSurfelArea;

/*Cosine lobe coeff*/
#define SH_cosLobe_C0 0.886226925f // sqrt(pi)/2 
#define SH_cosLobe_C1 1.02332671f // sqrt(pi/3) 

#define PI 3.1415926f

vec4 evalCosineLobeToDir(vec3 dir) 
{
	//f00, f-11, f01, f11
	return vec4(SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x );
}

float calculateBlockingPotential(vec3 dir, vec3 normal)
{
	return clamp((vSurfelArea * clamp(dot(normal, dir), 0.0, 1.0)) / (uGridSpacing.x * uGridSpacing.x), 0.0, 1.0); //It is probability so 0.0 - 1.0
}

void main()
{
	vec3 lightDir = normalize(uLightDirection - vRSMTexel.position); //Both are in world space
	float blockingPotential = calculateBlockingPotential(lightDir, vRSMTexel.normal);

	oGeometry = evalCosineLobeToDir(vRSMTexel.normal) * blockingPotential;
}
