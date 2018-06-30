#version 450 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

layout(binding = 3) uniform sampler2D uDepthTexture;
layout(binding = 1) uniform sampler2D uNormalTexture;
layout(binding = 5) uniform sampler2D uNoiseTexture;

uniform mat4 uProjection;
uniform mat4 uInverseProjection;
uniform vec3 uSamples[64];
uniform int uKernelSize = 64;
uniform float uRadius = 0.5;
uniform float uBias = 0.025;
uniform float uStrength = 1.0;

const float NEAR_PLANE = 0.1;
const float FAR_PLANE = 3000.0;

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

float getLinearDepth(float depth)
{
	return NEAR_PLANE * FAR_PLANE / (FAR_PLANE - depth * (FAR_PLANE - NEAR_PLANE));
}

void main()
{
	vec2 texSize = textureSize(uDepthTexture, 0);
	vec2 noiseScale = vec2(texSize * 0.25);
	vec2 texCoord = vTexCoord;//gl_FragCoord.xy /  texSize;
	
	float depth = texture(uDepthTexture, texCoord).r;
	vec4 clipSpacePos = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 viewSpacePos = uInverseProjection * clipSpacePos;
	viewSpacePos /= viewSpacePos.w;
	
    vec3 fragPos = viewSpacePos.xyz;
    vec3 N = decode(texture(uNormalTexture, texCoord).xy);
    vec3 randomVec = texture(uNoiseTexture, texCoord * noiseScale).xyz;
	randomVec.z = 0.0;
	randomVec = normalize(randomVec);
	
    vec3 tangent = normalize(randomVec - N * dot(randomVec, N));
    vec3 bitangent = cross(N, tangent);
    mat3 TBN = mat3(tangent, bitangent, N);
	
    float occlusion = 0.0;
    for(int i = 0; i < uKernelSize; ++i)
    {
        vec3 samplePos = TBN * uSamples[i]; // from tangent to view-space
        samplePos = samplePos * uRadius + fragPos; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = uProjection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(uDepthTexture, offset.xy).r;
		vec4 sampleClipSpacePos = vec4(offset.xy * 2.0 - 1.0, sampleDepth * 2.0 - 1.0, 1.0);
		vec4 sampleViewSpacePos = uInverseProjection * sampleClipSpacePos;
		sampleViewSpacePos /= sampleViewSpacePos.w;
		sampleDepth = sampleViewSpacePos.z;
        
        // range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + uBias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - min((occlusion / uKernelSize) * uStrength, 1.0);
    
    oColor = vec4(occlusion, 0.0, 0.0, 0.0);
}