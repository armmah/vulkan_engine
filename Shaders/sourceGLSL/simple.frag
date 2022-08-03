#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 viewDirection;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

void main() 
{
    vec4 color = vec4(1.0);//texture(texSampler, fragTexCoord);

    // fragNormal 
    // viewDirection
    float attenuation = dot(fragNormal, viewDirection);

    outColor = vec4(1.0, 1.0, 1.0, 1.0) * attenuation;
}