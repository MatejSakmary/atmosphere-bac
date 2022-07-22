layout (set = 2, binding = 0) uniform CloudsParametersBuffer
{
    vec4 shapeNoiseWeights;
    vec4 detailNoiseWeights;
    float detailNoiseMultiplier;
    float minBounds;
    float maxBounds;
    float cloudsScale;
    float detailScale;
    float densityOffset;
    float densityMultiplier;
    int sampleCount;
    int sampleCountToSun;
    float lightAbsTowardsSun;
    float lightAbsThroughCloud;
    float darknessThreshold;
    int debug;
    vec4 phaseParams;
} cloudsParameters;