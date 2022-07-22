#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <memory>
#include <cmath>
#include <cstring>
#include <iostream>

#include "VulkanDevice.h"
#include "VulkanBuffer.h"

#include "stb_image.h"
#include "tinyexr.h"

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
    VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t depth);
    
class VulkanImage
{
    public:
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;
        VkFormat format;

        uint32_t mipLevels;

        VulkanImage(std::shared_ptr<VulkanDevice> device,uint32_t width, uint32_t height, uint32_t mipLevels, 
            VkSampleCountFlagBits numSamples, VkFormat format,VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImageAspectFlags aspectFlags, uint32_t depth = 1);

        VulkanImage(std::shared_ptr<VulkanDevice>, const std::string &texturePath, bool isEXR = false);

        ~VulkanImage();

        void TransitionImageLayout(VkFormat format,VkImageLayout oldLayout, 
            VkImageLayout newLayout, uint32_t mipLevels);

        void CopyBufferToImage( VulkanBuffer &buffer, uint32_t width, uint32_t height);
    
    private:
        std::shared_ptr<VulkanDevice> device;

        void CreateImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, 
            VkSampleCountFlagBits numSamples, VkFormat format,VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImageAspectFlags aspectFlags);

        bool HasStencilComponent(VkFormat format);

        void GenerateMipmaps(VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};