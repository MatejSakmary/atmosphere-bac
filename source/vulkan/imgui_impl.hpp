#pragma once 

#include <stdexcept>
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
#include <glm/gtc/type_ptr.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "vulkan_device.hpp"
#include "vulkan_swapchain.hpp"
#include "camera.hpp"
#include "buffer_defines.hpp"
#include "model/sky_model.hpp"


class ImGuiImpl
{
    public:
        VkRenderPass imguiRenderPass;

    ImGuiImpl(VkInstance instance, std::shared_ptr<VulkanDevice> &device, GLFWwindow* window, 
            std::unique_ptr<VulkanSwapChain> &swapchain);
    
    ~ImGuiImpl();
    
    VkCommandBuffer PrepareNewFrame(uint32_t imageIndex, VkFramebuffer framebuffer,
        Camera *camera, PostProcessParamsBuffer &postParams, AtmosphereParametersBuffer &atmoParams,
        CloudsParametersBuffer &cloudParams, std::array<uint64_t, 60> &measurements, glm::vec2 extent);

    private:
        bool showPostProcessWindow;
        bool showAtmosphereParamsWindow;
        bool showCloudParamsWindow;

        uint32_t imageCount;
        VkDescriptorPool imguiDSPool;
        VkCommandPool imGuiCommandPool;
        std::shared_ptr<VulkanDevice> vDevice;
        /* This does not need to be a vector, but I am preparing for multiple frames in 
           flight */
        std::vector<VkCommandBuffer> imGuiCommandBuffers;
};