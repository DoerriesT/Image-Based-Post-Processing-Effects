#version 330 core

out vec4 oFragColor;

in vec3 vNormal;

uniform vec3 uLightDir;
uniform float uRayleighBrightness;
uniform float uMieBrightness;
uniform float uMieDistribution;
uniform float uSpotBrightness;
uniform float uSurfaceHeight;
uniform int uStepCount;
uniform vec3 uIntensity;
uniform float uScatterStrength;
uniform float uRayleighStrength;
uniform float uMieStrength;
uniform float uRayleighCollectionPower;
uniform float uMieCollectionPower;

const vec3 Kr = vec3(0.18867780436772762, 0.4978442963618773, 0.6616065586417131);

float phase(float alpha, float g)
{
    float a = 3.0 * (1.0 - g * g);
    float b = 2.0 * (2.0 + g * g);
    float c = 1.0 + alpha * alpha;
    float d = pow(1.0 + g * g - 2.0 * g * alpha, 1.5);
    return (a / b) * (c / d);
}

float atmosphericDepth(vec3 position, vec3 dir)
{
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, position);
    float c = dot(position, position) - 1.0;
    float det = b * b - 4.0 * a * c;
    float detSqrt = sqrt(det);
    float q = (-b - detSqrt) / 2.0;
    float t1 = c/q;
    return t1;
}

float horizonExtinction(vec3 position, vec3 dir, float radius)
{
    float u = dot(dir, -position);
    if(u < 0.0)
	{
        return 1.0;
    }
    vec3 near = position + u*dir;
    if(length(near) < radius)
	{
        return 0.0;
    }
    else
	{
        vec3 v2 = normalize(near) * radius - position;
        float diff = acos(dot(normalize(v2), dir));
        return smoothstep(0.0, 1.0, pow(diff * 2.0, 3.0));
    }
}

vec3 absorb(float dist, vec3 color, float factor)
{
    return color - color * pow(Kr, vec3(factor / dist));
}

void main()
{		
    vec3 camDir = normalize(vNormal);
	
	float alpha = max(dot(camDir, uLightDir), 0.0);
	
	float rayleighFactor = phase(alpha, -0.01) * uRayleighBrightness;
	float mieFactor = phase(alpha, uMieDistribution) * uMieBrightness;
	float spot = smoothstep(0.0, 15.0, phase(alpha, 0.9995)) * uSpotBrightness;

	vec3 camPosition = vec3(0.0, uSurfaceHeight, 0.0);
	float camDepth = atmosphericDepth(camPosition, camDir);
	float stepLength = camDepth / float(uStepCount);
	
	float camExtinction = horizonExtinction(camPosition, camDir, uSurfaceHeight - 0.15);

	vec3 rayleighCollected = vec3(0.0);
	vec3 mieCollected = vec3(0.0);
	
	for(int i = 0; i < uStepCount; ++i)
	{
		float sampleDistance = stepLength * float(i);
		vec3 position = camPosition + camDir * sampleDistance;
		float extinction = horizonExtinction(position, uLightDir, uSurfaceHeight - 0.35);
		float sampleDepth = atmosphericDepth(position, uLightDir);
		vec3 influx = absorb(sampleDepth, uIntensity, uScatterStrength) * extinction;
		rayleighCollected += absorb(sampleDistance, Kr * influx, uRayleighStrength);
		mieCollected += absorb(sampleDistance, influx, uMieStrength);
	}
	
	rayleighCollected = (rayleighCollected * camExtinction * pow(camDepth, uRayleighCollectionPower)) / float(uStepCount);
	mieCollected = (mieCollected * camExtinction *  pow(camDepth, uMieCollectionPower)) / float(uStepCount);

	vec3 color = vec3(spot * mieCollected + mieFactor * mieCollected + rayleighFactor * rayleighCollected);	
	oFragColor = vec4(color * 0.2, 1.0);
}

