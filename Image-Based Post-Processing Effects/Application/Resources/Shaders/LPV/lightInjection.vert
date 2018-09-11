#version 450 core

layout(binding = 0) uniform sampler2D uFluxTexture;
layout(binding = 1) uniform sampler2D uNormalTexture;

uniform mat4 uInvViewProjection;
uniform int uRsmWidth = 512;
uniform vec3 uGridOrigin;
uniform vec3 uGridSize;
uniform vec2 uGridSpacing; // spacing, 1.0 / spacing

struct RSMTexel
{
	vec3 flux;
	vec3 normal;
	vec3 position;
};

out flat vec3 vFlux;
out flat vec3 vNormal;
out flat vec3 vGridCell;

vec3 getGridPos(vec3 position)
{
	return (position - uGridOrigin) * uGridSpacing.y;
}

RSMTexel getRSM(ivec2 texCoord)
{
	vec3 flux = texelFetch(uFluxTexture, texCoord, 0).rgb;
	vec4 normalDepth = texelFetch(uNormalTexture, texCoord, 0).xyzw;
	
	vec4 position = vec4(vec3(vec2(texCoord) / textureSize(uFluxTexture, 0).xy, normalDepth.w) * 2.0 - 1.0, 1.0);
	position = uInvViewProjection * position;
	
	// shift position by half cellsize in normal direction
	return RSMTexel(flux, normalDepth.xyz, uGridSpacing.x * 0.5 * normalDepth.xyz + (position.xyz / position.w));
}


void main()
{
	ivec2 rsmCoord = ivec2(gl_VertexID % uRsmWidth, gl_VertexID / uRsmWidth);
	
	RSMTexel texel = getRSM(rsmCoord);
	ivec3 gridCell = ivec3(getGridPos(texel.position));

	vFlux = texel.flux;
	vNormal = texel.normal;
	vGridCell = gridCell;


	vec2 outputCoord = vec2(vGridCell.z * uGridSize.x + vGridCell.x, vGridCell.y) + vec2(0.5);
	outputCoord /= vec2(uGridSize.x * uGridSize.z, uGridSize.y);

	gl_Position = vec4(outputCoord * 2.0 - 1.0, 0.0, 1.0);
}