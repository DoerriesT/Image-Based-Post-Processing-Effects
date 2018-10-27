#version 450 core

layout(location = 0) out vec4 o01;
layout(location = 1) out vec4 o11;
layout(location = 2) out vec4 o10;
layout(location = 3) out vec4 o00;
layout(location = 4) out vec4 o21;
layout(location = 5) out vec4 o31;
layout(location = 6) out vec4 o30;
layout(location = 7) out vec4 o20;

layout(binding = 0) uniform sampler2D uDepthTexture;

uniform vec2 uOffset;

void main()
{
	vec2 texCoord = floor(gl_FragCoord.xy) * 4.0 + uOffset + 0.5f;
	texCoord *= 1.0 / textureSize(uDepthTexture);
	
	vec4 S0 = textureGather(uDepthTexture, texCoord, 0);
	vec4 S1 = textureGatherOffset(uDepthTexture, texCoord, ivec2(2, 0), 0);
	
	o01 = S0.x;
	o11 = S0.y;
	o10 = S0.z;
	o00 = S0.w;
	o21 = S1.x;
	o31 = S1.y;
	o30 = S1.z;
	o20 = S1.w;
}