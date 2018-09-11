#version 450 core

layout(binding = 0) uniform sampler2D uNormalTexture;

uniform mat4 uInvViewProjection;
uniform int uRsmWidth = 512;
uniform vec3 uGridOrigin;
uniform vec3 uGridSize;
uniform vec2 uGridSpacing; // spacing, 1.0 / spacing

struct RSMTexel
{
	vec3 normal;
	vec3 position;
};

out RSMTexel vRSMTexel;
out float vSurfelArea;

RSMTexel getRSM(ivec2 texCoord)
{
	vec4 normalDepth = texelFetch(uNormalTexture, texCoord, 0).xyzw;
	
	vec4 position = vec4(vec3(vec2(texCoord) / textureSize(uNormalTexture, 0).xy, normalDepth.w) * 2.0 - 1.0, 1.0);
	position = uInvViewProjection * position;
	
	// shift position by half cellsize and by half cellsize in normal direction
	return RSMTexel(normalDepth.xyz, uGridSpacing.x * 0.5 * normalDepth.xyz + (position.xyz / position.w) + vec3(uGridSpacing.x * 0.5));
}

vec3 getGridPos(vec3 position)
{
	return (position - uGridOrigin) * uGridSpacing.y;
}

void main()
{
	ivec2 rsmCoord = ivec2(gl_VertexID % uRsmWidth, gl_VertexID / uRsmWidth);
	vRSMTexel = getRSM(rsmCoord);
	ivec3 gridCell = ivec3(getGridPos(vRSMTexel.position));

	vec2 outputCoord = vec2(gridCell.z * uGridSize.x + gridCell.x, gridCell.y) + vec2(0.5);
	outputCoord /= vec2(uGridSize.x * uGridSize.z, uGridSize.y);

	gl_Position = vec4(outputCoord * 2.0 - 1.0, 0.0, 1.0);
	
	float worldSpaceTexelWidth = (uGridSize.x / uRsmWidth) * uGridSpacing.y;
    vSurfelArea = worldSpaceTexelWidth * worldSpaceTexelWidth;
}
