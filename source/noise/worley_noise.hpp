/* Heavily based on:
        https://github.com/SebLague/Clouds/blob/master/Assets/Scripts/Clouds/Noise/NoiseGenerator.cs 
*/

#include <iostream>
#include <random>
#include <vector>
#include <array>

/* Force alignment of glm data types to respect the alignment required
   by Vulkan NOTE: this does not cover nested data structures in that case
   I need to use alignas(16) prefix */
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
/* GLM uses OpenGL default of -1,1 for the perspective projection so
   I need to force it to use Vulkan default which is 0,1 */
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "vulkan/vulkan_image.hpp"
#include "vulkan/vulkan_device.hpp"
#include "vulkan/vulkan_buffer.hpp"
#include "vulkan/vulkan_pipeline.hpp"

#include <vulkan/vulkan.h>

struct WorleyParamsBufferObject
{
    alignas(16) glm::ivec3 numDivisions;
    alignas(16) glm::ivec3 texDimensions;
    alignas(16) glm::vec4 targetChannels;
    alignas(4) float persistence;
};

struct MinMaxParamsBufferObject
{
    alignas(4) int minVal;
    alignas(4) int maxVal;
};

struct WorleyNoiseCreateParams
{
    glm::ivec3 numDivisionsRChannel;
    glm::ivec3 numDivisionsGChannel;
    glm::ivec3 numDivisionsBChannel;
    glm::ivec3 numDivisionsAChannel;
    float persistenceRChannel;
    float persistenceGChannel;
    float persistenceBChannel;
    float persistenceAChannel;
};

struct PerChannelData
{
    std::unique_ptr<VulkanImage> oneChannelNoiseImage;
    std::unique_ptr<VulkanBuffer> pointsABuffer;
    std::unique_ptr<VulkanBuffer> pointsBBuffer;
    std::unique_ptr<VulkanBuffer> pointsCBuffer;
    std::unique_ptr<VulkanBuffer> minMaxBuffer;
    std::unique_ptr<VulkanBuffer> worleyParamsUBO;

    std::vector<glm::vec3> pointsABufferCPU;
    std::vector<glm::vec3> pointsBBufferCPU;
    std::vector<glm::vec3> pointsCBufferCPU;

    VkDescriptorSetLayout worleyNoiseDSLayout;
    VkDescriptorSet worleyNoiseDS;

    WorleyParamsBufferObject worleyParams;
};

class WorleyNoise3D
{
    public:
        std::unique_ptr<VulkanImage> noiseImage; 

        WorleyNoise3D(glm::ivec3 texDimensions, WorleyNoiseCreateParams params, 
            std::shared_ptr<VulkanDevice> device, VkDescriptorPool pool);

        void generateNoise();

    private:
        std::array<PerChannelData, 4> perChannelData;
        /* ==================== Random number generator ====================*/
        std::mt19937 mt;
        std::uniform_real_distribution<float> distribution;

        /* ==================== Texture parameters =========================*/
        glm::vec3 texDimensions;
        WorleyNoiseCreateParams params;

        /* ==================== Shared Vulkan Resources ====================*/
        VkDescriptorSetLayout normalizeNoiseDSLayout;
        VkDescriptorSet normalizeNoiseDS;
        std::shared_ptr<VulkanDevice> device;
        std::unique_ptr<VulkanPipeline> worleyNoisePipeline;
        std::unique_ptr<VulkanPipeline> normalizeNoisePipeline;
        VkCommandBuffer worleyNoiseCommadBuffer;

        VkSemaphore channelFinishedSemaphore;

        float getRandNum();
        void generateWorleyPointsBuffer(std::vector<glm::vec3> &buffer, int numDivisions);
        void copyCPUBufferIntoGPUBuffer(void *cpuData, std::unique_ptr<VulkanBuffer> &GPUBuffer,
            VkDeviceSize bufferSize);
};