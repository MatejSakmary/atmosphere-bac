#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <stdexcept>
#include <iostream>

#include "vulkan_device.hpp"

class VulkanBuffer
{
    public:
        std::shared_ptr<VulkanDevice> device;
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;

        VulkanBuffer(std::shared_ptr<VulkanDevice> device ,VkDeviceSize size, VkBufferUsageFlags usage, 
            VkMemoryPropertyFlags properties);

        ~VulkanBuffer();
        void CopyIntoBuffer(VulkanBuffer &srcBuffer, VkDeviceSize size);
};