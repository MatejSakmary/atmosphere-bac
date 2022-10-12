#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <array>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_device.hpp"
#include "vulkan_debug.hpp"
#include "vulkan_swapchain.hpp"
#include "vulkan_pipeline.hpp"
#include "primitives.hpp"
#include "model/sky_model.hpp"
#include "camera.hpp"
#include "imgui_impl.hpp"
#include "buffer_defines.hpp"
#include "noise/worley_noise.hpp"

#include "imgui.h"

#define MAX_FRAMES_IN_FLIGHT 1

/* Validation layers */
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

struct FrameData{
    std::unordered_map<std::string, VkDescriptorSet> descriptorSets;
    std::unordered_map<std::string, std::shared_ptr<VulkanBuffer>> buffers;
    std::unordered_map<std::string, std::shared_ptr<VulkanImage>> images;
    std::unordered_map<std::string, VkFramebuffer> framebuffers;
    std::unordered_map<std::string, VkCommandBuffer> commandBuffers;
    VkQueryPool querryPool;
    // quering with availability
    std::array<uint64_t, 30 * 2> timestamps;
    VkFence inFlightFence;
};

class Renderer
{
public:
    Camera* camera;
    bool framebufferResized;
    std::shared_ptr<VulkanDevice> vDevice;
    Renderer(GLFWwindow *window, bool enableValidation);
    ~Renderer();

    void drawFrame();

private:
    bool validationEnabled;
    size_t currentFrame = 0;
    std::array<FrameData, 3> perFrameData;
    /*========================== Frame independent data ===============================*/
    AtmosphereParametersBuffer atmoParamsBuffer;
    PostProcessParamsBuffer postProcessParamsBuffer;
    CloudsParametersBuffer cloudsParamsBuffer;
    std::unique_ptr<WorleyNoise3D> noise;
    std::unique_ptr<WorleyNoise3D> detailNoise;

    std::unique_ptr<ImGuiImpl> imguiImpl;
    
    std::unique_ptr<VulkanSwapChain> vSwapChain;

    std::unordered_map<std::string, VkDescriptorSetLayout> descriptorLayouts;
    std::unordered_map<std::string, VkDescriptorSet> frameSharedDS;
    std::unordered_map<std::string, std::shared_ptr<VulkanImage>> frameSharedImages;

    /* Pipelines */
    std::unique_ptr<VulkanPipeline> finalPassPipeline;
    std::unique_ptr<VulkanPipeline> terrainPassPipeline;
    std::unique_ptr<VulkanPipeline> cloudsPassPipeline;
    std::unique_ptr<VulkanPipeline> farSkyPassPipeline;
    std::unique_ptr<VulkanPipeline> aePerspectivePassPipeline;
    /* Compute Pipelines */
    std::unique_ptr<VulkanPipeline> transmittanceLUTPipeline;
    std::unique_ptr<VulkanPipeline> multiscatteringLUTPipeline;
    std::unique_ptr<VulkanPipeline> skyViewLUTPipeline;
    std::unique_ptr<VulkanPipeline> AEPerspectiveLUTPipeline;

    std::unique_ptr<VulkanPipeline> histogramPipeline;
    std::unique_ptr<VulkanPipeline> sumHistogramPipeline;

    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;

    GLFWwindow *window;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkRenderPass renderPass;
    VkRenderPass hdrBackbufferPass;

    VkSemaphore postProcessReadySemaphore;
    VkSampler AEPerspectiveSampler;
    VkSampler cloudsSampler;
    VkSampler skyViewLUTSampler;
    VkSampler terrainTexturesSampler;
    VkSampler depthTextureSampler;

    VkDescriptorPool descriptorPool;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    /**
     * Create instance of Vulkan -> used as a per application access point for all 
     * other Vulkan functionality
     * @param enableValidation if TRUE validation layers for Vulkan will be enabled 
     *      -> Vulkan checks for errors - decreases performance
     */
    void createInstance(bool enableValidation);
    void createPreset(int presetNum);

    /**
     * Create window surface using glfw functionality
     *  -> throws runtime error on failure
     */
    void loadAssets();
    void createSurface();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipelines();
    void createFramebuffers(); 
    void createAttachments();
    void createSampler();
    void createPrimitivesBuffers();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    void updateUniformBuffer(uint32_t currentImage);
    void recreateSwapChain();
    void cleanupSwapchain();
    
    // Compute
    void prepareTextureTargets(uint32_t width, uint32_t height, VkFormat format);
    void prepareComputeUniformBuffers();
    void createComputePipelines();
    void createComputeCommandBuffer();
    void createComputeSyncObjects();
    void buildComputeCommandBuffers();
    void updateComputeUniformBuffer();

    // Timestamps
    void createQuerryPool();
};