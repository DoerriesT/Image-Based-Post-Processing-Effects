#version 330 core

layout (location = 0) in vec2 aPosition;  
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords; 
layout (location = 3) in vec3 aTangent; 
layout (location = 4) in vec3 aBitangent; 

out vec2 vTexCoord;
out vec3 vWorldPos;

uniform sampler2D uDisplacementTexture;
uniform mat4 uProjection;
uniform mat4 uView;
uniform vec3 uCamPos;
uniform float uWaterLevel;
uniform float uPositionSnapDistance = 200.0/300.0; // patchsize / vertices
  
void main()
{
	float attenuation = max(abs(aPosition.x), abs(aPosition.y));

	vec2 viewSpacePos = mat2(0.707, -0.707, 0.707, 0.707) * aPosition.xy;

	vec3 cameraForwardDir = uView[2].xyz;
	cameraForwardDir.z = -cameraForwardDir.z;

	mat3 viewToWorld = mat3(uView);

	// select better projection axis if front angle too steep
	float frontDotUpAxis = abs( cameraForwardDir.y );
	viewToWorld[1] = mix(viewToWorld[1], uView[2].xyz, int(frontDotUpAxis > 0.75f));
	viewToWorld[2] = mix(viewToWorld[2], -uView[1].xyz, int(frontDotUpAxis > 0.75f));

	// displace grid forward, scale it and transform it to worldspace
	vec3 worldSpacePos = (-200.0 * vec3(viewSpacePos.x, 0.0, viewSpacePos.y + 0.7)) * viewToWorld;

	float cameraDistance = length(worldSpacePos);
	worldSpacePos /= cameraDistance;
	
	// scale grid depending on camera height
	cameraDistance *= 1.0 + abs(uCamPos.y + uWaterLevel) / 32.0;

	// snap far end of grid to horizon
	float t = dot(normalize(cameraForwardDir.xz), worldSpacePos.xz);
	cameraDistance = mix(cameraDistance, 3000.0 * 2.0, int(sign(t) * attenuation > 0.98));

	// snap grid in increments to camera position
	vec2 positionOffset = uCamPos.xz;
	positionOffset.xy -= fract(positionOffset.xy / uPositionSnapDistance) * uPositionSnapDistance;

	worldSpacePos *= cameraDistance;
	worldSpacePos.xz += positionOffset;

	// add some height near horizon to hide artifacts
	worldSpacePos.y = uWaterLevel + (4.5 * int(attenuation > 0.98));

	vTexCoord = worldSpacePos.xz * 0.01;

	float fDisplaceAtten = clamp(0.0, 1.0, cameraDistance * 0.001 );
	fDisplaceAtten *= fDisplaceAtten;

	vec3 displacement = texture(uDisplacementTexture, vTexCoord).xyz;

	worldSpacePos += (1.0 - attenuation * attenuation) * displacement * 12.0;
	vWorldPos = worldSpacePos;

	gl_Position = uProjection * uView * vec4(worldSpacePos, 1.0);
}