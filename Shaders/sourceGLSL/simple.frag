#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragLightSpacePos;
layout(location = 4) in vec4 bias_ambient;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2DShadow shadowDepthSampler;
layout(set = 3, binding = 0) uniform sampler2D mainTexSampler;

#define DEPTH_BIAS bias_ambient.x
#define NORMAL_BIAS bias_ambient.y
#define AMBIENT bias_ambient.z

float getOccluderDepth(sampler2DShadow shadowMap, vec2 uvs, float pixelDepth)
{
    return texture(shadowMap, vec3(uvs.xy, pixelDepth)).r;// - DEPTH_BIAS;
}

float filteredSampleVisibilityOcclusion(vec2 projCoords, float pixelDepth)
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowDepthSampler, 0);
    for(float x = -1.0; x <= 1.0; ++x)
    {
        for(float y = -1.0; y <= 1.0; ++y)
        {
            shadow += getOccluderDepth(shadowDepthSampler, projCoords.xy + vec2(x, y) * texelSize, pixelDepth);
        }    
    }

    return shadow / 9.0;
}

float sampleVisibilityOcclusion(vec2 projCoords, float pixelDepth)
{
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = getOccluderDepth(shadowDepthSampler, projCoords.xy, pixelDepth);
    // check whether current frag pos is in shadow
    return pixelDepth > closestDepth ? 1.0 : 0.0;
}

float shadowCalculation(vec4 fragLightSpacePos)
{
    // perform perspective divide
    vec3 projCoords = fragLightSpacePos.xyz / fragLightSpacePos.w;
    // transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    // get depth of current fragment from light's perspective
    float currentDepth = 1.0 - projCoords.z;

    return filteredSampleVisibilityOcclusion(projCoords.xy, projCoords.z);
    return sampleVisibilityOcclusion(projCoords.xy, projCoords.z);
}

void main()
{
    vec4 color = texture(mainTexSampler, fragTexCoord);
    float shadowMap = shadowCalculation(fragLightSpacePos);
    float attenuation = mix(1.0, AMBIENT, shadowMap);

    outColor = color * attenuation;
}