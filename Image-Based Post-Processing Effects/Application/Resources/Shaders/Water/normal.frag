#version 330 core

layout(location = 0) out vec4 oNormal;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uDisplacementTexture;

uniform float uNormalStrength = 2.5;

vec2 encode (vec3 n)
{
    float f = sqrt(8.0 * n.z + 8.0);
    return n.xy / f + 0.5;
}

void main()
{
	// 02 -- 12 -- 22
	// |	 |     |
	// 01 -- c  -- 21
	// |     |     |
	// 00 -- 10 -- 20
	
	vec2 texelSize = vec2(1.0 / textureSize(uDisplacementTexture, 0));
	
	float s02 = texture(uDisplacementTexture, vTexCoord + vec2(-texelSize.x, texelSize.y)).y;
	vec3 s12 = texture(uDisplacementTexture, vTexCoord + vec2(0.0, texelSize.y)).xyz;
	float s22 = texture(uDisplacementTexture, vTexCoord + texelSize).y;
	float s01 = texture(uDisplacementTexture, vTexCoord + vec2(-texelSize.x, 0.0)).y;
	vec3 s21 = texture(uDisplacementTexture, vTexCoord + vec2(texelSize.x, 0.0)).xyz;
	float s00 = texture(uDisplacementTexture, vTexCoord - texelSize).y;
	float s10 = texture(uDisplacementTexture, vTexCoord + vec2(0.0, -texelSize.y)).y;
	float s20 = texture(uDisplacementTexture, vTexCoord + vec2(texelSize.x, -texelSize.y)).y;


	
	// The Sobel X kernel is:
	//
	// [ 1.0  0.0  -1.0 ]
	// [ 2.0  0.0  -2.0 ]
	// [ 1.0  0.0  -1.0 ]
	
	float x = s00 - s20 + 2.0f * s01 - 2.0f * s21.y + s02 - s22;
				
	// The Sobel Y kernel is:
	//
	// [  1.0    2.0    1.0 ]
	// [  0.0    0.0    0.0 ]
	// [ -1.0   -2.0   -1.0 ]
	
	float y = s00 + 2.0f * s10 + s20 - s02 - 2.0f * s12.y - s22;

	vec3 N = normalize(vec3(x, 1.0/uNormalStrength, y));
	
	oNormal = vec4(encode(N), 0.0 , 1.0);
}