#version 330 core

layout(location = 0) out vec4 oBlur;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uColorTexture;

uniform bool uBlur;

vec4 blur(vec2 pixelSize)
{
	vec4 sum = vec4(0.0);

	vec2 upLeft = vec2(-pixelSize.x, pixelSize.y);
	vec2 downRight = vec2(pixelSize.x, -pixelSize.y);
	vec2 up = vec2(0.0, pixelSize.y);
	vec2 left = vec2(-pixelSize.x, 0.0);

	sum += 0.125 * texture(uColorTexture,  2 * upLeft		+ vTexCoord).rgba;
	sum += 0.25  * texture(uColorTexture,  2 * up			+ vTexCoord).rgba;
	sum += 0.125 * texture(uColorTexture,  2 * pixelSize	+ vTexCoord).rgba;
	sum += 0.5   * texture(uColorTexture,  1 * upLeft		+ vTexCoord).rgba;
	sum += 0.5   * texture(uColorTexture,  1 * pixelSize	+ vTexCoord).rgba;
	sum += 0.25  * texture(uColorTexture,  2 * left			+ vTexCoord).rgba;
	sum += 0.5   * texture(uColorTexture,					  vTexCoord).rgba;
	sum += 0.25  * texture(uColorTexture,  2 * -left		+ vTexCoord).rgba;
	sum += 0.5   * texture(uColorTexture,  1 * -pixelSize	+ vTexCoord).rgba;
	sum += 0.5   * texture(uColorTexture,  1 * -upLeft		+ vTexCoord).rgba;
	sum += 0.125 * texture(uColorTexture,  2 * -pixelSize	+ vTexCoord).rgba;
	sum += 0.25  * texture(uColorTexture,  2 * -up			+ vTexCoord).rgba;
	sum += 0.125 * texture(uColorTexture,  2 * -upLeft		+ vTexCoord).rgba;

	return sum * 0.25;
}

void main()
{
	vec2 pixelSize = vec2(1.0/textureSize(uColorTexture, 0));
	oBlur = blur(pixelSize);
}