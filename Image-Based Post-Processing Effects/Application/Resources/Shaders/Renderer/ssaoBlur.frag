#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

layout(binding = 6) uniform sampler2D uInputTexture;

uniform int uBlurSize = 4; // size of noise texture

void main() 
{
	vec2 texSize = textureSize(uInputTexture, 0);
	vec2 texCoord = gl_FragCoord.xy /  texSize;
	vec2 texelSize = vec2(1.0 / texSize);
	float result = 0.0;
	vec2 hlim = vec2(float(-uBlurSize) * 0.5 + 0.5);
	for (int i = 0; i < uBlurSize; ++i) 
	{
		for (int j = 0; j < uBlurSize; ++j) 
		{
			vec2 offset = (hlim + vec2(float(i), float(j))) * texelSize;
			result += texture(uInputTexture, texCoord + offset).r;
		}
	}
 
	oColor = vec4(result / float(uBlurSize * uBlurSize), 0.0, 0.0, 0.0);
}