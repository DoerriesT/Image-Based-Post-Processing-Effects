#version 330 core

out vec4 oFragColor;
in vec2 vTexCoord;

uniform sampler2D uScreenTexture; // full resolution source texture
uniform sampler2D uBloomTexture;
uniform sampler2D uLensFlareTex; // input from the blur stage
uniform sampler2D uLensDirtTex; // full resolution dirt texture
uniform sampler2D uLensStarTex; // diffraction starburst texture
uniform mat3 uLensStarMatrix; // transforms texcoords
uniform bool uLensFlares;
uniform bool uBloom;
uniform float uBloomStrength = 0.1;
uniform float uBloomDirtStrength = 0.5;
uniform float uExposure = 1.0;


const float A = 0.15; // shoulder strength
const float B = 0.50; // linear strength
const float C = 0.10; // linear angle
const float D = 0.20; // toe strength
const float E = 0.02; // toe numerator
const float F = 0.30; // toe denominator
const vec3 W = vec3(11.2); // linear white point value

vec3 uncharted2Tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main()
{
	vec3 color = texture(uScreenTexture, vTexCoord).rgb;

	if (uBloom)
	{
		vec3 lensMod = texture(uLensDirtTex, vTexCoord).rgb;
		vec3 bloom = texture(uBloomTexture, vTexCoord).rgb * uBloomStrength;
		bloom = mix(bloom, bloom * (vec3(1.0) + lensMod), uBloomDirtStrength);
		color += bloom;
	}

	if (uLensFlares)
	{
		vec3 lensMod = texture(uLensDirtTex, vTexCoord).rgb;
		vec2 lensStarTexCoord = (uLensStarMatrix * vec3(vTexCoord, 1.0)).xy;
		lensStarTexCoord = clamp(lensStarTexCoord, vec2(0.0), vec2(1.0));
		lensMod += texture(uLensStarTex, lensStarTexCoord).rgb;
		vec3 lensFlare = texture(uLensFlareTex, vTexCoord).rgb * lensMod;
		color.rgb += vec3(1.0) - exp(-lensFlare * 8.0 * dot(lensFlare, vec3(0.299, 0.587, 0.114)));
		//color.rgb += lensFlare;
	}
	
	// HDR tonemapping
	float exposureBias = 2.0;
	//float exposure = 0.18/max((clamp(adaptedLum, -14.0, 16.0) - exposureBias), 0.00001);
	color *= uExposure;
	
	color = uncharted2Tonemap(exposureBias * color);
	vec3 whiteScale = 1.0/uncharted2Tonemap(W);
	color *= whiteScale;
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

	oFragColor = vec4(color, dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}