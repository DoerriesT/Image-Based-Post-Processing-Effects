#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec2 vTexCoord;
out vec3 vWorldPos;
out vec4 vDerivatives;

uniform sampler2D uDisplacementTexture;
uniform mat4 uProjection;
uniform mat4 uView;
uniform vec3 uCamPos;
uniform float uWaterLevel;

bool toWorldSpace(vec3 screenSpacePos, out vec3 worldSpacePos, out vec4 derivatives)
{
	vec4 vertex = vec4(screenSpacePos, 1);
    
	mat4 invProjection = inverse(uProjection);
	mat4 invView = inverse(uView);

    vec3 cameraDir = normalize(invProjection * vertex ).xyz;
    vec3 worldDir = (invView * vec4(cameraDir, 0)).xyz;
    
    if (worldDir.y == 0)
    {
		return false;
	}

    float t = (uCamPos.y - uWaterLevel) / -worldDir.y;
    
    if (t < 0)
    {
		return false;
	}
    
    worldSpacePos = uCamPos + t * worldDir;
    worldSpacePos.y = uWaterLevel;

	// x-direction
	vec4 vertexX = vec4(screenSpacePos + vec3(1.0/150.0, 0.0, 0.0), 1.0);
	cameraDir = normalize(invProjection * vertexX ).xyz;
    worldDir = (invView * vec4(cameraDir, 0)).xyz;
	t = (uCamPos.y - uWaterLevel) / -worldDir.y;
	vec3 worldSpacePosX = uCamPos + t * worldDir;
	worldSpacePosX.y = uWaterLevel;

	derivatives.xy = worldSpacePosX.xz - worldSpacePos.xz;

	// y-direction
	vec4 vertexY = vec4(screenSpacePos + vec3(0.0, 1.0/150.0, 0.0), 1.0);
	cameraDir = normalize(invProjection * vertexY ).xyz;
    worldDir = (invView * vec4(cameraDir, 0)).xyz;
	t = (uCamPos.y - uWaterLevel) / -worldDir.y;
	vec3 worldSpacePosY = uCamPos + t * worldDir;
	worldSpacePosY.y = uWaterLevel;

	derivatives.zw = worldSpacePosY.xz - worldSpacePos.xz;
    
    return true;
}

void main() {
    
    float fact = 0.01;
    
    for(int i = 0; i < 3; ++i)
	{
		vec3 pos = vec3(0.0);
		vec4 derivatives = vec4(0.0);
		if(toWorldSpace(gl_in[i].gl_Position.xyz, pos, derivatives))
		{
			derivatives *= 0.001;
			vDerivatives = derivatives;
			vTexCoord = pos.xz/100.0;
			vec4 disp = textureGrad(uDisplacementTexture, vTexCoord, derivatives.xy, derivatives.zw);
			pos += disp.xyz * 10;
			vWorldPos = pos;
			gl_Position = uProjection * uView * vec4(pos, 1);
    
			EmitVertex();
      }
    }
 
    EndPrimitive();
}