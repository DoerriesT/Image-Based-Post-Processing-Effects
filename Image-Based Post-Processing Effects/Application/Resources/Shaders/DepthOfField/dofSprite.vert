#version 450 core

layout (location = 0) in vec2 aPosition;

layout(binding = 0) uniform sampler2D uCocTexture;
layout(binding = 1) uniform sampler2D uColorTexture;
layout(binding = 2) uniform sampler2D uDepthTexture;

uniform int uWidth;
uniform int uHeight;

out flat float vNear;
out flat vec4 vColor;
out vec2 vTexCoord;

void main()
{	
	vec2 pixelPos = vec2(gl_InstanceID % uWidth, gl_InstanceID / uWidth);
	vec2 texCoord = pixelPos / vec2(uWidth, uHeight);
	vec2 cocVals = texture(uCocTexture, texCoord).xy;
	
	float near = cocVals.x > cocVals.y ? 1.0 : 0.0;
	float coc = max(cocVals.x, cocVals.y) * 0.5;

	vColor = vec4(texture(uColorTexture, texCoord).rgb, 1.0 / (coc * coc));
	vTexCoord = aPosition * 0.5 + 0.5;
	vNear = float(near == 1.0);
	
	// scale by coc
	vec2 position = aPosition * coc;
	
	// move to pixel position
	position += pixelPos;
	
	// scale to pixel size
	position *= (1.0 / vec2(uWidth, uHeight));
	
	// screen space
	position = position * vec2(1.0, 2.0) - vec2(near, 1.0);
	
    gl_Position = vec4(position, coc < 1.0 ? -2.0 : 0.0, 1.0);
}