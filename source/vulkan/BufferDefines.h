#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
/* GLM uses OpenGL default of -1,1 for the perspective projection so
   I need to force it to use Vulkan default which is 0,1 */
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct UniformBufferObject
{
    /*NOTE: it's better to be to be explicit about alignment that way it's
            harder to get caught on specifics*/
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(4) float time;
    alignas(16) glm::mat4 lHviewProj;
};

struct PostProcessParamsBuffer
{
    alignas(4) float minimumLuminance;
    alignas(4) float maximumLuminance;
    alignas(4) float timeDelta;
    alignas(4) float lumAdaptTau;

    // only when using Reinhard
    alignas(4) float whitepoint;

    // only when using Uchimura
    alignas(4) float maxDisplayBrigtness;
    alignas(4) float contrast;
    alignas(4) float linearSectionStart;
    alignas(4) float linearSectionLength;
    alignas(4) float black;
    alignas(4) float pedestal;

    // only when using Lottes
    alignas(4) float a;
    alignas(4) float d;
    alignas(4) float hdrMax;
    alignas(4) float midIn;
    alignas(4) float midOut;

    alignas(4) int tonemapCurve;
    alignas(8) glm::vec2 texDimensions;
};

struct CloudsParametersBuffer
{
    alignas(16) glm::vec4 shapeNoiseWeights;
    alignas(16) glm::vec4 detailNoiseWeights;
    alignas(4)  float detailNoiseMultiplier;
    alignas(4)  float minBounds;
    alignas(4)  float maxBounds; 
    alignas(4)  float cloudsScale;
    alignas(4)  float detailScale;
    alignas(4)  float densityOffset;
    alignas(4)  float densityMultiplier;
    alignas(4)  int sampleCount;
    alignas(4)  int sampleCountToSun;
    alignas(4)  float lightAbsTowardsSun;
    alignas(4)  float lightAbsThroughCloud;
    alignas(4)  float darknessThreshold;
    alignas(4)  int debug;
    alignas(16) glm::vec4 phaseParams;
};