#version 330

in vec2 vTexCoord;

out vec4 oColor;

float phase(float alpha, float g)
{
    float a = 3.0 * (1.0 - g * g);
    float b = 2.0 * (2.0 + g * g);
    float c = 1.0 + alpha * alpha;
    float d = pow(1.0 + g * g - 2.0 * g * alpha, 1.5);
    return (a / b) * (c / d);
}

void main(void)
{
	vec3 pos = vec3(vTexCoord.xy, 0.0);
	pos -= 0.5;
	pos *= 2.0;
	pos.z = 1.0;
	
	float alpha = max(dot(normalize(pos), vec3(0.0, 0.0, 1.0)), 0.0);
	
	vec4 sun = smoothstep(0.0, 15.0, phase(alpha, 0.9995)) * vec4(10.0, 10.0, 8.0, 1.0);
	oColor = sun;
}