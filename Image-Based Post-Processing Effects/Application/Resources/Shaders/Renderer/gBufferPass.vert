#version 330 core

layout (location = 0) in vec3 aPosition;  
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords; 
layout (location = 3) in vec3 aTangent; 
layout (location = 4) in vec3 aBitangent; 
  
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec4 vCurrentPos;
out vec4 vPrevPos;

uniform mat4 uPrevTransform;
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform vec4 uAtlasData; // x = cols, y = rows, z = texOffsetX, w = texOffsetY


void main()
{
	// Support for texture atlas, update texture coordinates
    float x = (aTexCoords.x / uAtlasData.x + uAtlasData.z);
    float y = (aTexCoords.y / uAtlasData.y + uAtlasData.w);
    vTexCoord = vec2(x, y);

	mat3 modelMatrix = mat3(uModelMatrix);

    vNormal = modelMatrix * aNormal;
	vTangent = modelMatrix * aTangent;
	vBitangent = modelMatrix * aBitangent;

	vPrevPos = uPrevTransform * vec4(aPosition, 1.0);
	vCurrentPos = uModelViewProjectionMatrix * vec4(aPosition, 1.0);
	gl_Position = vCurrentPos;
}