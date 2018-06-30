#version 330 core

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 oTildeHktDx;
layout(location = 1) out vec4 oTildeHktDy;
layout(location = 2) out vec4 oTildeHktDz;

layout (pixel_center_integer) in vec4 gl_FragCoord;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uTildeH0kTexture;
layout(binding = 1) uniform sampler2D uTildeH0minusKTexture;

uniform int uN;
uniform int uL;
uniform float uTime;

const float g = 9.81;

struct Complex
{
	float real;
	float im;
};

Complex mul(Complex c0, Complex c1)
{
	Complex c;
	c.real = c0.real * c1.real - c0.im * c1.im;
	c.im = c0.real * c1.im + c0.im * c1.real;
	return c;
}

Complex add(Complex c0, Complex c1)
{
	Complex c;
	c.real = c0.real + c1.real;
	c.im = c0.im + c1.im;
	return c;
}

Complex conj(Complex c)
{
	Complex cConj = Complex(c.real, -c.im);
	return cConj;
}

void main() 
{
	vec2 x = gl_FragCoord.xy - float(uN) * 0.5;
	vec2 k = vec2(2.0 * M_PI * x.x/uL, 2.0 * M_PI * x.y/uL);
	
	float mag = length(k);
	mag = max(mag, 0.00001);
	
	float w = sqrt(g * mag);
	
	vec2 tildeH0kValues = texture(uTildeH0kTexture, vTexCoord).rg;
	Complex fourierCmp = Complex(tildeH0kValues.x, tildeH0kValues.y);
	
	vec2 tildeH0minusKValues = texture(uTildeH0minusKTexture, vTexCoord).rg;
	Complex fourierCmpConj = conj(Complex(tildeH0minusKValues.x, tildeH0minusKValues.y));
	
	float cosWT = cos(w * uTime);
	float sinWT = sin(w * uTime);
	
	// euler formula
	Complex expIWT = Complex(cosWT, sinWT);
	Complex expIWTInv = Complex(cosWT, -sinWT);
	
	// dy
	Complex hktDy = add(mul(fourierCmp, expIWT), mul(fourierCmpConj, expIWTInv));
	
	// dx
	Complex dx = Complex(0.0, -k.x / mag);
	Complex hktDx = mul(dx, hktDy);
	
	// dz
	Complex dy = Complex(0.0, -k.y / mag);
	Complex hktDz = mul(dy, hktDy);
	
	oTildeHktDy = vec4(hktDy.real, hktDy.im, 0.0, 1.0);
	oTildeHktDx = vec4(hktDx.real, hktDx.im, 0.0, 1.0);
	oTildeHktDz = vec4(hktDz.real, hktDz.im, 0.0, 1.0);
}

