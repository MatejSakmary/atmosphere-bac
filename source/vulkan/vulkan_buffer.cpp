#include "vulkan_buffer.hpp"

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties) : device{device}
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    /* Just like images in swap chain buffers can be owned by specific queue family*/
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("VULKAN_BUFFER::CONSTRUCTOR::Failed to create buffer");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->device, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(device->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("VULKAN_BUFFER::CONSTRUCTOR::Failed to allocate buffer memory");
    }
    
    /* Associate this memory with the buffer, 4th parameter -> offset within the region of memory*/
    vkBindBufferMemory(device->device, buffer, bufferMemory, 0);
}

VulkanBuffer::~VulkanBuffer()
{
    vkDestroyBuffer(device->device, buffer, nullptr);
    vkFreeMemory(device->device, bufferMemory, nullptr);
}

void VulkanBuffer::CopyIntoBuffer(VulkanBuffer &srcBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = device->BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // OPTIONAL
    copyRegion.dstOffset = 0; // OPTIONAL
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, buffer, 1, &copyRegion);

    device->EndSingleTimeCommands(commandBuffer);
}