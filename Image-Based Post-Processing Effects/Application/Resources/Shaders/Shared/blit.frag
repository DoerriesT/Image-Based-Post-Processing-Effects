#version 330 core

out vec4 oFragColor;
in vec2 vTexCoord;

uniform sampler2D uScreenTexture;
uniform float uScale = 1.0;
uniform bool uPower = false;
uniform float uPowerValue;
uniform bool uRedToWhite = false;
uniform bool uNormalMode = false;
uniform mat3 uInvViewMatrix;

vec3 decode (vec2 enc)
{
    vec2 fenc = enc * 4.0 - 2.0;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0 - f * 0.25);
    vec3 n;
    n.xy = fenc * g;
    n.z = 1.0 -f * 0.5;
    return n;
}

void main()
{
	oFragColor = uRedToWhite ? texture(uScreenTexture, vTexCoord).rrra : texture(uScreenTexture, vTexCoord).rgba;

	if(uNormalMode)
	{
		vec3 normal = decode(oFragColor.rg);
		oFragColor.rgb = uInvViewMatrix * normal;
	}

	oFragColor.rgb *= uScale;

	if(uPower)
	{
		oFragColor.rgb = pow(oFragColor.rgb, vec3(uPowerValue));
	}
}