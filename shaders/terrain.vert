#version 450
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec3 worldPosition;


/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
layout(set = 2, binding = 0) uniform sampler2D heightMapSampler;

void main() 
{
    float scale = 0.07;
    float height = texture(heightMapSampler, inTexCoord).r * scale; 
    mat4 PVMmatrix = commonParameters.proj * commonParameters.view * commonParameters.model;
    outTexCoord = vec2(inPosition.xy);
    worldPosition = (commonParameters.model * vec4(inPosition.x, inPosition.y, height, 1.0)).rgb;
    gl_Position = PVMmatrix * vec4(inPosition.x,inPosition.y,height, 1.0);
}