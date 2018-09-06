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
	position = (position - uGridOrigin) * uGridSpacing.y;
	return clamp(round(position), uGridOrigin, uGridSize * uGridSpacing.x + uGridOrigin);
}

RSMTexel getRSM(ivec2 texCoord)
{
	vec3 flux = texelFetch(uFluxTexture, texCoord, 0).rgb;
	vec4 normalDepth = texelFetch(uNormalTexture, texCoord, 0).xyzw;
	
	vec4 position = vec4(vec3(texCoord / textureSize(uFluxTexture, 0).xy, normalDepth.w) * 2.0 - 1.0, 1.0);
	position = uInvViewProjection * position;
	
	return RSMTexel(flux, normalDepth.xyz, 0.5 * normalDepth.xyz + (position.xyz / position.w));
}


void main()
{
	ivec2 rsmCoord = ivec2(gl_VertexID % uRsmWidth, gl_VertexID / uRsmWidth);
	
	// choose the brightest texel
	vec3 chosenGridCell = ivec3(0);
	{
		float maxLum = 0.0;
	
		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				ivec2 texCoord = rsmCoord + ivec2(i, j);
				RSMTexel texel = getRSM(texCoord);
				float curTexLum = dot(texel.flux, vec3(0.299, 0.587, 0.114));
				if (curTexLum > maxLum)
				{
					chosenGridCell = getGridPos(texel.position);
					maxLum = curTexLum;
				}
			}
		}
	}
	
	// filter
	RSMTexel result = RSMTexel(vec3(0.0), vec3(0.0), vec3(0.0));
	
	float samples = 0.0;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			ivec2 texCoord = rsmCoord + ivec2(i, j);
			RSMTexel texel = getRSM(texCoord);
			vec3 texelGridCell = getGridPos(texel.position);
			vec3 dGrid = texelGridCell - chosenGridCell;
			if (dot(dGrid, dGrid) < 3)
			{
				result.flux += texel.flux;
				result.normal += texel.normal;
				result.position += texel.position;
				++samples;
			}
		}
	}
	
	// normalize
	if(samples > 0.0)
	{
		float rcpSamples = 1.0 / samples;
		result.flux *= 0.25;
		result.normal *= rcpSamples;
		result.position *= rcpSamples;
	}
	
	vFlux = result.flux;
	vNormal = result.normal;
	vGridCell = getGridPos(result.position);
	
	gl_Position = vec4(vec2(vGridCell.z * uGridSize.x + vGridCell.x, vGridCell.y) / uGridSize.xy * 2.0 - 1.0, 0.0, 1.0);
}