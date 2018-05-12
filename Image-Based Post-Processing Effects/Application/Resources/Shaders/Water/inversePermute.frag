#version 330 core

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 oDisplacement;

layout (pixel_center_integer) in vec4 gl_FragCoord;
in vec2 vTexCoord;

uniform sampler2D uInputXTexture;
uniform sampler2D uInputYTexture;
uniform sampler2D uInputZTexture;
uniform int uN;
uniform float uChoppiness = -0.65;

void main() 
{
	vec2 x = gl_FragCoord.xy;
	float perms[2] = float[]( 1.0, -1.0 );
	int index = int(mod((int(x.x + x.y)), 2.0));
	float perm = perms[index];
	
	float hX = texture(uInputXTexture, vTexCoord).r;
	float hY = texture(uInputYTexture, vTexCoord).r;
	float hZ = texture(uInputZTexture, vTexCoord).r;

	vec3 h = vec3(hX, hY, hZ);
	h = perm * (h / float(uN * uN));

	// choppiness
	h.xz *= uChoppiness;
	
	oDisplacement = vec4(h, 0.0);
}

