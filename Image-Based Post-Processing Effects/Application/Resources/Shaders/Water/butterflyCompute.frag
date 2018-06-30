#version 330 core

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 oOutputXTexture;
layout(location = 1) out vec4 oOutputYTexture;
layout(location = 2) out vec4 oOutputZTexture;

layout (pixel_center_integer) in vec4 gl_FragCoord;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uButterflyTexture;
layout(binding = 1) uniform sampler2D uInputXTexture;
layout(binding = 2) uniform sampler2D uInputYTexture;
layout(binding = 3) uniform sampler2D uInputZTexture;

uniform int uN;
uniform int uStage;
uniform int uStages;
uniform int uDirection;

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

float getNormalizedCoord(float coord, float maxValue)
{
	return (coord / maxValue) + 1.0 / (maxValue * 2.0);
}

void horizontalButterflies()
{
	vec2 x = gl_FragCoord.xy;
	vec4 data = texture(uButterflyTexture, vec2(getNormalizedCoord(uStage, float(uStages)), vTexCoord.x)).rgba;
	vec2 pX_ = texture(uInputXTexture, vec2(getNormalizedCoord(data.z, float(uN)), vTexCoord.y)).rg;
	vec2 qX_ = texture(uInputXTexture, vec2(getNormalizedCoord(data.w, float(uN)), vTexCoord.y)).rg;
	vec2 pY_ = texture(uInputYTexture, vec2(getNormalizedCoord(data.z, float(uN)), vTexCoord.y)).rg;
	vec2 qY_ = texture(uInputYTexture, vec2(getNormalizedCoord(data.w, float(uN)), vTexCoord.y)).rg;
	vec2 pZ_ = texture(uInputZTexture, vec2(getNormalizedCoord(data.z, float(uN)), vTexCoord.y)).rg;
	vec2 qZ_ = texture(uInputZTexture, vec2(getNormalizedCoord(data.w, float(uN)), vTexCoord.y)).rg;
	vec2 w_ = data.xy;
	
	Complex pX = Complex(pX_.x, pX_.y);
	Complex qX = Complex(qX_.x, qX_.y);
	Complex pY = Complex(pY_.x, pY_.y);
	Complex qY = Complex(qY_.x, qY_.y);
	Complex pZ = Complex(pZ_.x, pZ_.y);
	Complex qZ = Complex(qZ_.x, qZ_.y);
	Complex w = Complex(w_.x, w_.y);
	
	Complex HX = add(pX, mul(w, qX));
	oOutputXTexture = vec4(HX.real, HX.im, 0.0, 1.0);

	Complex HY = add(pY, mul(w, qY));
	oOutputYTexture = vec4(HY.real, HY.im, 0.0, 1.0);

	Complex HZ = add(pZ, mul(w, qZ));
	oOutputZTexture = vec4(HZ.real, HZ.im, 0.0, 1.0);
}

void verticalButterflies()
{
	vec2 x = gl_FragCoord.xy;
	vec4 data = texture(uButterflyTexture, vec2(getNormalizedCoord(uStage, float(uStages)), vTexCoord.y)).rgba;
	vec2 pX_ = texture(uInputXTexture, vec2(vTexCoord.x, getNormalizedCoord(data.z, float(uN)))).rg;
	vec2 qX_ = texture(uInputXTexture, vec2(vTexCoord.x, getNormalizedCoord(data.w, float(uN)))).rg;
	vec2 pY_ = texture(uInputYTexture, vec2(vTexCoord.x, getNormalizedCoord(data.z, float(uN)))).rg;
	vec2 qY_ = texture(uInputYTexture, vec2(vTexCoord.x, getNormalizedCoord(data.w, float(uN)))).rg;
	vec2 pZ_ = texture(uInputZTexture, vec2(vTexCoord.x, getNormalizedCoord(data.z, float(uN)))).rg;
	vec2 qZ_ = texture(uInputZTexture, vec2(vTexCoord.x, getNormalizedCoord(data.w, float(uN)))).rg;
	vec2 w_ = data.xy;
	
	Complex pX = Complex(pX_.x, pX_.y);
	Complex qX = Complex(qX_.x, qX_.y);
	Complex pY = Complex(pY_.x, pY_.y);
	Complex qY = Complex(qY_.x, qY_.y);
	Complex pZ = Complex(pZ_.x, pZ_.y);
	Complex qZ = Complex(qZ_.x, qZ_.y);
	Complex w = Complex(w_.x, w_.y);
	
	Complex HX = add(pX, mul(w, qX));
	oOutputXTexture = vec4(HX.real, HX.im, 0.0, 1.0);

	Complex HY = add(pY, mul(w, qY));
	oOutputYTexture = vec4(HY.real, HY.im, 0.0, 1.0);

	Complex HZ = add(pZ, mul(w, qZ));
	oOutputZTexture = vec4(HZ.real, HZ.im, 0.0, 1.0);
}

void main() 
{
	if (uDirection == 0)
	{
		horizontalButterflies();
	}
	else
	{
		verticalButterflies();
	}
}

