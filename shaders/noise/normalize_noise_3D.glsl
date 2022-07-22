#version 450

struct Vector{
    float x, y, z;
};

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

layout (std430, set = 0, binding = 0) buffer minMaxBuffer { int minVal; int maxVal; } minMax[4];
layout (set = 0, binding = 1, r16f) uniform image3D perChannelNoise [4];
layout (set = 0, binding = 2, rgba16f) uniform image3D resultNoise;

const float minMaxPrecision = 10000000.0;
void main()
{
    float resultAsFloatArr [4];
    for(int i = 0; i < 4; i++)
    {
        float val = imageLoad(perChannelNoise[i], ivec3(gl_GlobalInvocationID.xyz)).r;
        float minValFloat = float(minMax[i].minVal) / float(minMaxPrecision);
        float maxValFloat = float(minMax[i].maxVal) / float(minMaxPrecision);
        float normalizedVal = (val - minValFloat) / (maxValFloat - minValFloat);
        resultAsFloatArr[i] = normalizedVal;
    }
    vec4 result = vec4(resultAsFloatArr[0], resultAsFloatArr[1], resultAsFloatArr[2], resultAsFloatArr[3]); 
    imageStore(resultNoise, ivec3(gl_GlobalInvocationID.xyz), result);
}