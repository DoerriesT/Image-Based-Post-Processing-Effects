#version 450 core

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 oButterflyTexture;

layout (pixel_center_integer) in vec4 gl_FragCoord;
in vec2 vTexCoord;

uniform int uJ[512];
uniform int uN;

const float g = 9.81;

struct Complex
{
	float real;
	float im;
};

void main() 
{
	vec2 x =  gl_FragCoord.xy;
	float k = mod(x.y * (float(uN) / pow(2.0 , x.x + 1.0)), uN);
	Complex twiddle = Complex(cos(2.0 * M_PI * k / float(uN)), sin(2.0 * M_PI * k / float(uN)));
	
	int butterflySpan = int(pow(2, x.x));
	
	int butterflyWing;
	
	if (mod(x.y, pow(2.0, x.x + 1.0)) < pow(2.0, x.x))
	{
		butterflyWing = 1;
	}
	else
	{
		butterflyWing = 0;
	}
	
	// first stage, bit reversed indices
	if(x.x == 0.0)
	{
		// top butterfly wing
		if (butterflyWing == 1)
		{
			oButterflyTexture = vec4(twiddle.real, twiddle.im, uJ[int(x.y)], uJ[int(x.y + 1.0)]);
		}
		// bottom butterfly wing
		else
		{
			oButterflyTexture = vec4(twiddle.real, twiddle.im, uJ[int(x.y - 1.0)], uJ[int(x.y)]);
		}
	}
	// second to log2(N) stage
	else
	{
		// top butterfly wing
		if (butterflyWing == 1)
		{
			oButterflyTexture = vec4(twiddle.real, twiddle.im, x.y, x.y + butterflySpan);
		}
		// bottom butterfly wing
		else
		{
			oButterflyTexture = vec4(twiddle.real, twiddle.im, x.y - butterflySpan, x.y);
		}
	}
}

