/* Parts of code take from https://github.com/SebLague/Clouds/blob/master/Assets/Scripts/Clouds/Shaders/Clouds.shader */
#version 450

#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"

layout (location = 0) out vec4 outColor;
layout (location = 0) in vec2 inUV;


/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout (set = 2, binding = 0) uniform sampler2D texSampler; 
layout (input_attachment_index = 0, set = 3, binding = 0) uniform subpassInput depthInput; 

vec3 sunWithBloom(vec3 worldDir, vec3 sunDir)
{
    const float sunSolidAngle = 1.0 * PI / 180.0;
    const float minSunCosTheta = cos(sunSolidAngle);

    float cosTheta = dot(worldDir, sunDir);
    if(cosTheta >= minSunCosTheta) {return vec3(0.5) ;}
    float offset = minSunCosTheta - cosTheta;
    float gaussianBloom = exp(-offset * 50000.0) * 0.5;
    float invBloom = 1.0/(0.02 + offset * 300.0) * 0.01;
    return vec3(gaussianBloom + invBloom);
}
/* One unit in global space should be 100 meters in camera coords */
const float cameraScale = 0.1;
void main() 
{
    const vec3 camera = atmosphereParameters.camera_position;
    vec3 sun_direction = atmosphereParameters.sun_direction;
    vec2 atmosphereBoundaries = vec2(
        atmosphereParameters.bottom_radius, 
        atmosphereParameters.top_radius);

    mat4 invViewProjMat = inverse(commonParameters.proj * commonParameters.view);
    vec2 pixPos = inUV;
    vec3 ClipSpace = vec3(pixPos*vec2(2.0) - vec2(1.0), 1.0);
    
    vec4 Hpos = invViewProjMat * vec4(ClipSpace, 1.0);

    vec3 WorldDir = normalize(Hpos.xyz/Hpos.w - camera); 
    vec3 WorldPos = camera * cameraScale + vec3(0,0, atmosphereParameters.bottom_radius);

    float viewHeight = length(WorldPos);
    vec3 L = vec3(0.0,0.0,0.0);

    float depth = subpassLoad(depthInput).r;

    if(depth == 1.0)
    {
        vec2 uv;
        vec3 UpVector = normalize(WorldPos);
        float viewZenithAngle = acos(dot(WorldDir, UpVector));

        float lightViewAngle = acos(dot(normalize(vec3(sun_direction.xy, 0.0)), normalize(vec3(WorldDir.xy, 0.0))));
        bool IntersectGround = raySphereIntersectNearest(WorldPos, WorldDir, vec3(0.0, 0.0, 0.0),
            atmosphereParameters.bottom_radius) >= 0.0;

        uv = SkyViewLutParamsToUv(IntersectGround, vec2(viewZenithAngle,lightViewAngle), viewHeight,
            atmosphereBoundaries, atmosphereParameters.SkyViewTexDimensions);

        L += vec3(texture(texSampler, vec2(uv.x, uv.y)).rgb);

        if(!IntersectGround)
        {
            L += sunWithBloom(WorldDir, sun_direction);
        };

        outColor = vec4(L, 1.0);
    }
    else{
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}