#version 450 core

out vec4 oFragColor;
in vec2 vTexCoord;

uniform sampler2D uScreenTexture;
uniform float uScale = 1.0;
uniform bool uPower = false;
uniform float uPowerValue;
uniform bool uRedToWhite = false;
uniform bool uNormalMode = false;
uniform mat3 uInvViewMatrix;
uniform bool uYCRMode = false;

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

vec2 signNotZero(vec2 v) 
{
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

/** Returns a unit vector. Argument o is an octahedral vector packed via octEncode,
    on the [-1, +1] square*/
vec3 octDecode(vec2 o) 
{
    vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
    if (v.z < 0.0) 
	{
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    }
    return normalize(v);
}

vec2 unorm8x3ToSnorm12x2(vec3 u)
{
	u *= 255.0;
	u.y *= (1.0 / 16.0);
	vec2 s = vec2(u.x * 16.0 + floor(u.y), fract(u.y) * (16.0 * 256.0) + u.z);
	return clamp(s * (1.0 / 2047.0) - 1.0, vec2(-1.0), vec2(1.0));
}

vec3 YCoCg2RGB(vec3 c)
{
	c.y -= 0.5;
	c.z -= 0.5;
	return vec3(c.r + c.g - c.b, c.r + c.b, c.r - c.g - c.b);
}

const float THRESH=30./255.;

float edgeFilter(vec2 center, vec2 a0, vec2 a1, vec2 a2, vec2 a3)
{ 
	vec4 lum = vec4(a0.x, a1.x , a2.x, a3.x);
	vec4 w = 1.0 - step(THRESH, abs(lum - center.x)); 
	float W = w.x + w.y + w.z + w.w;
	//Handle the special case where all the weights are zero.
	//In HDR scenes it's better to set the chrominance to zero. 
	//Here we just use the chrominance of the first neighbor.
	w.x = (W == 0.0) ? 1.0 : w.x;  
	W = (W == 0.0) ? 1.0 : W;  

	return (w.x * a0.y + w.y * a1.y + w.z * a2.y + w.w * a3.y) * (1.0 / W);
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy / textureSize(uScreenTexture, 0);
	oFragColor = uRedToWhite ? texture(uScreenTexture, texCoord).rrra : texture(uScreenTexture, texCoord).rgba;

	if(uNormalMode)
	{
		vec3 normal = octDecode(unorm8x3ToSnorm12x2(oFragColor.xyz));
		oFragColor.rgb = uInvViewMatrix * normal;
	}
	else if (uYCRMode)
	{
		vec2 texelSize = 1.0 / textureSize(uScreenTexture, 0);
		vec2 a0 = texture(uScreenTexture, texCoord + vec2(texelSize.x, 0.0)).rg;
		vec2 a1 = texture(uScreenTexture, texCoord - vec2(texelSize.x, 0.0)).rg;
		vec2 a2 = texture(uScreenTexture, texCoord + vec2(0.0, texelSize.y)).rg;
		vec2 a3 = texture(uScreenTexture, texCoord - vec2(0.0, texelSize.y)).rg;	
		const float chroma = edgeFilter(oFragColor.rg, a0, a1, a2, a3);

		const ivec2 crd = ivec2(gl_FragCoord.xy);
		const bool pattern = ((crd.x & 1) == (crd.y & 1));
		
		vec3 albedo = vec3(oFragColor.rg, chroma);
		oFragColor.rgb = YCoCg2RGB(pattern ? albedo.rbg : albedo.rgb);
	}

	oFragColor.rgb *= uScale;

	if(uPower)
	{
		oFragColor.rgb = pow(oFragColor.rgb, vec3(uPowerValue));
	}
}