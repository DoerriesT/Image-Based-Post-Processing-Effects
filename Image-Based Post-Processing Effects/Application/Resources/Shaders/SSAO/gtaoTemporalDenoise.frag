#version 450 core

const float PI = 3.14159265;

in vec2 vTexCoord;

out vec4 oColor;

layout(binding=4) uniform sampler2D uVelocityTexture;
layout(binding=5) uniform sampler2D uPreviousTexture;
layout(binding=6) uniform sampler2D uInputTexture;

uniform float uFrameTime;

float calulateAlpha(float frameTime, float convergenceTime)
{
	return exp(-frameTime / convergenceTime);
}

void main(void)
{
	vec2 center = texelFetch(uInputTexture, ivec2(gl_FragCoord.xy), 0).xy;
	vec2 texelSize = 1.0 / textureSize(uInputTexture, 0).xy;
	
	float ao = center.x;
	float depth = center.y;
	
	// get previous frames ao value
	vec2 velocity = texture(uVelocityTexture, vTexCoord).rg;
	vec2 reprojectedCoord = vTexCoord - velocity;
	vec2 previousAo = texture(uPreviousTexture, reprojectedCoord).xy;

	// is the reprojected coordinate inside the frame?
	float insideFrame = float(reprojectedCoord.x < 1.0 && reprojectedCoord.y < 1.0 && reprojectedCoord.x >= 0.0 && reprojectedCoord.y >= 0.0);
	
	// based on depth, how likely describes the value the same point?
	float depthWeight = clamp(1.0 - ((depth - previousAo.y) / (depth * 0.01)), 0.0, 1.0);
	
	// based on velocity how likely is the coherence
	velocity *= textureSize(uVelocityTexture, 0).xy;
	float velocityWeight = clamp(1.0 - (length(velocity) / 40.0), 0.0, 1.0);
	
	// neighborhood clamping
	vec2 minMax = vec2(1e6, -1e6);
	
	// determine neighborhood
	{
		// top left
		{
			float S = texture(uInputTexture, 1.5 * vec2(-texelSize.x, texelSize.y) + vTexCoord).x;
			minMax.x = min(S, minMax.x);
			minMax.y = max(S, minMax.y);
		}
		// top right
		{
			float S = texture(uInputTexture, 1.5 * vec2(texelSize.x, texelSize.y) + vTexCoord).x;
			minMax.x = min(S, minMax.x);
			minMax.y = max(S, minMax.y);
		}
		// bottom left
		{
			float S = texture(uInputTexture, 1.5 * vec2(-texelSize.x, -texelSize.y) + vTexCoord).x;
			minMax.x = min(S, minMax.x);
			minMax.y = max(S, minMax.y);
		}
		// bottom right
		{
			float S = texture(uInputTexture, 1.5 * vec2(texelSize.x, -texelSize.y) + vTexCoord).x;
			minMax.x = min(S, minMax.x);
			minMax.y = max(S, minMax.y);
		}
		// center
		minMax.x = min(ao, minMax.x);
		minMax.y = max(ao, minMax.y);
	}
	
	// widen the min/max window based on velocity
	float window = velocityWeight * (minMax.y - minMax.x);
	minMax.x -= 1.5 * window;
	minMax.y += 1.5 * window;
	previousAo.x = clamp(previousAo.x, minMax.x, minMax.y);
	
	//float convergenceAlpha = calulateAlpha(uFrameTime, velocityWeight * 0.5 * (1.0 / 60.0) * 24.0);
	
	ao = mix(ao, previousAo.x, (23.0 / 24.0) * depthWeight * velocityWeight * insideFrame);
	
	oColor = vec4(ao, depth, 0.0, 1.0);
}