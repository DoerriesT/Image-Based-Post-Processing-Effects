#version 330 core

out vec4 oFragColor;

in vec3 vNormal;

uniform samplerCube uEnvironmentMap;

const float PI = 3.14159265359;

void main()
{		
    vec3 N = normalize(vNormal);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up = cross(N, right);
	mat3 TBN = mat3(right, up, N);
       
    float sampleDelta = 0.025;
    float samples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = TBN * tangentSample; 

            irradiance += texture(uEnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            ++samples;
        }
    }
    irradiance = PI * irradiance * (1.0 / samples);
    
    oFragColor = vec4(irradiance, 1.0);
}

