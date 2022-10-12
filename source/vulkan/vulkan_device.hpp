#pragma once

#include <optional>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() &&
               presentFamily.has_value() &&
               computeFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/* Required extensions */
/* TODO: This should be made as a parameter instead of const defined like this */
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

class VulkanDevice
{
public:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSampleCountFlagBits msaaSamples;

    QueueFamilyIndices familyIndices;
    VkCommandPool graphicsCommandPool;
    VkCommandPool computeCommandPool;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    VulkanDevice(const VkInstance &instance, const VkSurfaceKHR surface);
    ~VulkanDevice();

    void createCommandPool();
    VkCommandBuffer createGraphicsCommandBuffer();
    VkCommandBuffer createComputeCommandBuffer();
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface);


private:

    void pickPhysicalDevice(const VkInstance &instance, const VkSurfaceKHR surface);
    void createLogicalDevice(const VkSurfaceKHR surface);

    bool isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(const VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface);
};