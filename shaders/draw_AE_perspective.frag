#version 450

#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"

layout (location = 0) out vec4 outColor;
layout (location = 0) in vec2 inUV;

/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout (input_attachment_index = 0, set = 2, binding = 0) uniform subpassInput depthInput; 
layout (set = 3, binding = 0) uniform sampler3D AEPerspectiveSampler;

/* One unit in global space should be 100 meters in camera coords */
const float cameraScale = 0.1;
void main()
{
    vec3 cameraPosition = atmosphereParameters.camera_position;

    /* sample depth from depth map used in rendering the terrain  */
    float depth = subpassLoad(depthInput).r;
    // if(depth == 1.0)
    // {
    //     outColor = vec4(0.0, 0.0, 0.0, 1.0);
    //     return;
    // }
    /* Vulkans clip space range [-1, -1] - (1, 1) -> z is depth [0, 1] */
    vec3 clipSpace = vec3(inUV * vec2(2.0) - vec2(1.0), depth);
    /* inverse view projection matrix which gets us from clip space to world space */
    mat4 invViewProjMat = inverse(commonParameters.proj * commonParameters.view);
    vec4 hPos = invViewProjMat * vec4(clipSpace, 1.0);
    /* Get unit lenght ray from camera origin to processed fragment in world space */
    vec3 cameraRayWorld = normalize(hPos.xyz/hPos.w - cameraPosition);

    /* Get depth in world space */
    float realDepth = length(hPos.xyz/hPos.w - cameraPosition);

    float Slice = realDepth * cameraScale * (1.0 / 4.0);
    float Weight = 1.0;

    if (Slice < 0.5)
    {
        Weight = clamp(Slice * 2.0, 0.0, 1.0);
        Slice = 0.5;
    }
    float w = sqrt(Slice / atmosphereParameters.AEPerspectiveTexDimensions.z);
    vec4 APVal = Weight * texture(AEPerspectiveSampler, vec3(inUV, w));
    outColor = APVal;
    // outColor = vec4(realDepth, 0.0, 0.0, 0.0);
}