#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragLightSpacePos;
layout(location = 4) in vec4 bias_ambient;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D shadowDepthSampler;
layout(set = 3, binding = 0) uniform sampler2D mainTexSampler;

#define DEPTH_BIAS bias_ambient.x
#define NORMAL_BIAS bias_ambient.y
#define AMBIENT bias_ambient.z

float getOccluderDepth(sampler2D shadowMap, vec2 uvs)
{
    return 1.0 - texture(shadowMap, uvs).r - DEPTH_BIAS;
}

float filteredSampleVisibilityOcclusion(vec2 projCoords, float pixelDepth)
{
    float shadow = 0.0;
    vec2 texelSize = 2.6 / textureSize(shadowDepthSampler, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = getOccluderDepth(shadowDepthSampler, projCoords.xy + vec2(x, y) * texelSize);
            shadow += pixelDepth > pcfDepth ? 1.0 : 0.0;
        }    
    }
    return shadow / 9.0;
}

float sampleVisibilityOcclusion(vec2 projCoords, float pixelDepth)
{
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = getOccluderDepth(shadowDepthSampler, projCoords.xy);
    // check whether current frag pos is in shadow
    return pixelDepth > closestDepth ? 1.0 : 0.0;
}

float shadowCalculation(vec4 fragLightSpacePos)
{
    // perform perspective divide
    vec3 projCoords = (fragLightSpacePos.xyz + fragNormal * NORMAL_BIAS) / fragLightSpacePos.w;
    // transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    // get depth of current fragment from light's perspective
    float currentDepth = 1.0 - projCoords.z;

    return filteredSampleVisibilityOcclusion(projCoords.xy, currentDepth);
}

void main()
{
    vec4 color = texture(mainTexSampler, fragTexCoord);
    float shadowMap = shadowCalculation(fragLightSpacePos);
    float attenuation = mix(AMBIENT, 1.0, shadowMap);

    outColor = color * attenuation;
}