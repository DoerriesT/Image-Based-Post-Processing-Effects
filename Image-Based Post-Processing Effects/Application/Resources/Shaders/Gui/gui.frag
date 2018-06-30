#version 450 core

out vec4 oColor;

in vec2 vTexCoord;
in vec2 vPosition;
in vec4 vColor;
flat in int vShouldBlur;

layout(binding = 0) uniform sampler2D uTexture;
layout(binding = 1) uniform sampler2D uBlurTexture;

void main()
{
	if (vShouldBlur == 1)
	{
		const int sampleLod = 2;
		vec2 texelSize = 1.0 / (textureSize(uBlurTexture, 0) * 0.5);
		vec2 pos = (vPosition + 1.0) * 0.5;

		vec3 backColor = vec3(0.0);

		

		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, texelSize.y * -5.5), sampleLod).rgb * 0.014646;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, texelSize.y * -5.5), sampleLod).rgb * 0.018763;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, texelSize.y * -5.5), sampleLod).rgb * 0.021531;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0.0, texelSize.y * -5.5), sampleLod).rgb * 0.011142;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, texelSize.y * -5.5), sampleLod).rgb * 0.021531;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, texelSize.y * -5.5), sampleLod).rgb * 0.018763;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, texelSize.y * -5.5), sampleLod).rgb * 0.014646;
		
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, texelSize.y * -3.5), sampleLod).rgb * 0.018763;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, texelSize.y * -3.5), sampleLod).rgb * 0.024037;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, texelSize.y * -3.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0.0, texelSize.y * -3.5), sampleLod).rgb * 0.014274;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, texelSize.y * -3.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, texelSize.y * -3.5), sampleLod).rgb * 0.024037;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, texelSize.y * -3.5), sampleLod).rgb * 0.018763;
		
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, texelSize.y * -1.5), sampleLod).rgb * 0.021531;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, texelSize.y * -1.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, texelSize.y * -1.5), sampleLod).rgb * 0.031650;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0.0, texelSize.y * -1.5), sampleLod).rgb * 0.016380;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, texelSize.y * -1.5), sampleLod).rgb * 0.031650;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, texelSize.y * -1.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, texelSize.y * -1.5), sampleLod).rgb * 0.021531;
		                                                                               
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, 0.0), sampleLod).rgb * 0.011142;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, 0.0), sampleLod).rgb * 0.014274;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, 0.0), sampleLod).rgb * 0.016380;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0.0, 0.0), sampleLod).rgb * 0.008477;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, 0.0), sampleLod).rgb * 0.016380;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, 0.0), sampleLod).rgb * 0.014274;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, 0.0), sampleLod).rgb * 0.011142;
		                                                                                
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, texelSize.y * 1.5), sampleLod).rgb * 0.021531;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, texelSize.y * 1.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, texelSize.y * 1.5), sampleLod).rgb * 0.031650;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0., texelSize.y * 1.5), sampleLod).rgb * 0.016380;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, texelSize.y * 1.5), sampleLod).rgb * 0.031650;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, texelSize.y * 1.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, texelSize.y * 1.5), sampleLod).rgb * 0.021531;
		
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, texelSize.y * 3.5), sampleLod).rgb * 0.018763;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, texelSize.y * 3.5), sampleLod).rgb * 0.024037;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, texelSize.y * 3.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0.0, texelSize.y * 3.5), sampleLod).rgb * 0.014274;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, texelSize.y * 3.5), sampleLod).rgb * 0.027582;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, texelSize.y * 3.5), sampleLod).rgb * 0.024037;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, texelSize.y * 3.5), sampleLod).rgb * 0.018763;
		
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -5.5, texelSize.y * 5.5), sampleLod).rgb * 0.014646;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -3.5, texelSize.y * 5.5), sampleLod).rgb * 0.018763;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x * -1.5, texelSize.y * 5.5), sampleLod).rgb * 0.021531;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  0.0, texelSize.y * 5.5), sampleLod).rgb * 0.011142;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  1.5, texelSize.y * 5.5), sampleLod).rgb * 0.021531;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  3.5, texelSize.y * 5.5), sampleLod).rgb * 0.018763;
		backColor += textureLod(uBlurTexture, pos + vec2(texelSize.x *  5.5, texelSize.y * 5.5), sampleLod).rgb * 0.014646;


		oColor = vec4(vColor.rgb * vColor.a + backColor * (1.0 - vColor.a), 1.0) ;
		
	}
	else
	{
		oColor = vColor * texture(uTexture, vTexCoord);
	}
   
}