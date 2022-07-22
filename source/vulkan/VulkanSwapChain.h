#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <stdexcept>

#include "VulkanDevice.h"
#include "VulkanImage.h"

class VulkanSwapChain
{
    public:
        VkSwapchainKHR swapChain;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        uint32_t imageCount;
        uint32_t minImageCount;
        std::vector<VkImageView> swapChainImageViews;
        
        VulkanSwapChain(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, int width, int height);
        ~VulkanSwapChain();


    private:
        VkSurfaceKHR surface;
        std::shared_ptr<VulkanDevice> device;    

        std::vector<VkImage> swapChainImages;

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, int width, int height);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
};