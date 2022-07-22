#version 450
#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"

layout (location = 0) in vec2 outTexCoord;
layout (location = 1) in vec3 worldPosition;

layout (location = 0) out vec4 outColor;

/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout(set = 2, binding = 1) uniform sampler2D diffuseMapSampler;
layout(set = 2, binding = 2) uniform sampler2D normalMapSampler;
layout(set = 3, binding = 0, rgba16f) uniform readonly image2D transmittanceLUT;

/* One unit in global space should be 100 meters in camera coords */
const float cameraScale = 0.1;

void main()
{
    const vec3 base = vec3(124.0,141.0,76.0)/255.0;
    const vec3 rockColor = vec3(0.258, 0.260, 0.258);
    const vec3 hillColor = vec3(229.0,217.0,194.0)/255.0;
    const vec3 lowlandColor = vec3(181.0,186.0,97.0)/255.0;
    vec3 sun_direction = atmosphereParameters.sun_direction;
    /* Flip to point towards sun */
    sun_direction *= vec3(-1.0, -1.0, -1.0);
    
    vec3 normal = (texture(normalMapSampler, outTexCoord)).rgb;
    /* Remap from [0,1] to [-1,1] */
    normal = normalize(normal * 2.0 - vec3(1.0, 1.0, 1.0));
    /* Fix handness to match rest of application */
    normal = normal * vec3(-1.0, 1.0, -1.0);

    vec4 maskWeights = texture(diffuseMapSampler, outTexCoord);
    maskWeights /= max( dot(maskWeights, vec4(1.0, 1.0, 1.0, 1.0)), 1.0);
    vec3 texColor = maskWeights.a * base + maskWeights.b * rockColor +
        maskWeights.g * hillColor + maskWeights.r * lowlandColor;

    float diff = max(dot(normal, sun_direction), 0.0);

    vec3 realWorldPos = worldPosition * cameraScale + vec3(0.0, 0.0, atmosphereParameters.bottom_radius);
    float height = length(realWorldPos);
    vec3 upVector = realWorldPos / height;
    float viewZenithCosAngle = dot(sun_direction * vec3(-1.0), upVector);
    vec2 transLUTParams = vec2( height, viewZenithCosAngle);
    vec2 atmosphereBoundaries = vec2(atmosphereParameters.bottom_radius, atmosphereParameters.top_radius);
    vec2 transUV = TransmittanceLUTParamsToUv(transLUTParams, atmosphereBoundaries); 
    ivec2 transImageCoords = ivec2(transUV * atmosphereParameters.TransmittanceTexDimensions);
    vec3 transmittanceToSun = vec3(imageLoad(transmittanceLUT, transImageCoords).rgb);

    vec3 ambient = vec3(0.1, 0.1, 0.1) * texColor;
    vec3 diffuse = diff * texColor;
    outColor = vec4((ambient + diffuse).xyz * 0.05 * transmittanceToSun, 1.0);
}