#include <stdexcept>
#include <iostream>

#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(const VkInstance &instance, const VkSurfaceKHR surface)
{
    /* TODO: Check if this is really necessary */
    msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    pickPhysicalDevice(instance, surface);
    createLogicalDevice(surface);
}

VulkanDevice::~VulkanDevice()
{
    vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
    vkDestroyCommandPool(device, computeCommandPool, nullptr);
    vkDestroyDevice(device, nullptr);
}

void VulkanDevice::pickPhysicalDevice(const VkInstance &instance, const VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("VULKAN_DEVICE::PICK_PHYSICAL_DEVICE::failed \
            to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device, surface))
        {
            physicalDevice = device;
            /* TODO: This value should be held by renderer not device */
            msaaSamples = getMaxUsableSampleCount();
            std::cout << "VULKAN_DEVICE::PICK_PHYSICAL_DEVICE::Choosing " 
                      << msaaSamples << " msaa Samples" << std::endl;
            familyIndices = findQueueFamilies(device, surface);
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("VULKAN_DEVICE::PICK_PHYSICAL_DEVICE::failed to find a suitable GPU");
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "VULKAN_DEVICE::PICK_PHYSICAL_DEVICE::Choosing physical device "
              << properties.deviceName << " timestamp period is " <<
               properties.limits.timestampPeriod << std::endl;
}

bool VulkanDevice::isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    /* Querry for device features and check if anisotropic filtering is supported by device */
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(const VkPhysicalDevice device,
    const VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        /* Check for support of presentation capability */
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        if(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            if (queueFamily.timestampValidBits == 0)
            {
                throw std::runtime_error("VKDEVICE::FIND_QUEUE_FAMILIES::Timestamps\
                    not valid for compute shaders");
            }
            indices.computeFamily = i;
        }
        if (indices.isComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

bool VulkanDevice::checkDeviceExtensionSupport(const VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport(const VkPhysicalDevice device,
    const VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    /* query for basic surface capabilities
        -> takes into account physicalDevice and surface*/
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        /* resize the vector since we are using it as an array */
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        /* resize the vector since we are using it as an array */
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSampleCountFlagBits VulkanDevice::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = 
        physicalDeviceProperties.limits.framebufferColorSampleCounts
        &
        physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanDevice::createLogicalDevice(const VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
        {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value()};

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    /* Enable anisotropic filtering device feature */
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    /* Enable sample shading feature for the device */
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    /* Create logical device */
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("VULKAN_DEVIDCE::CREATE_LOGICAL_DEVICE::Failed \
            to create logical device");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat> &candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features)
{ 
    for (VkFormat format : candidates)
    {
        /* Contains three fields
            linearTilingFeatures: Use cases that are supported with linear tiling
            optimalTilingFeatures: Use cases that are supported with optimal tiling
            bufferFeatures: Use cases that are supported for buffers */
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        
        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (properties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (properties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("VULKAN_DEVICE::FIND_SUPPORTED_FORMAT::Failed \
        to find supported format");
}

void VulkanDevice::createCommandPool()
{

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = familyIndices.graphicsFamily.value();
    poolInfo.flags = 0;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("VULKAN_DEVICE::CREATE_COMMAND_POOL::Failed \
            to create command pool");
    }

    //Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = familyIndices.computeFamily.value();
    // cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.flags = 0;

    if (vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("VULKAN_DEVICE::CREATE_COMPUTE_COMMAND_POOL::Failed to \
            create command pool");
    } 
}

VkCommandBuffer VulkanDevice::createGraphicsCommandBuffer()
{
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphicsCommandPool;
    /* PRIMARY -> can be submitted to a queue for execution but cannot be called from other buffers
       SECONDARY -> cannot be submitted directly, but can be called from primary buffers*/
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("DEVICE::CREATE_COMMAND_BUFFERS::\
            Failed to allocate graphics command buffer");
    }
    return commandBuffer;
}

VkCommandBuffer VulkanDevice::createComputeCommandBuffer()
{
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = computeCommandPool;
    /* PRIMARY -> can be submitted to a queue for execution but cannot be called from other buffers
       SECONDARY -> cannot be submitted directly, but can be called from primary buffers*/
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("DEVICE::CREATE_COMMAND_BUFFERS::\
            Failed to allocate compute command buffer");
    }
    return commandBuffer;
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{ 
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        /* typeFilter is used to specify the bit field of memory types that
           are suitable -> we are iterating over them and checking if
           the corresponding bit is set */
        if (typeFilter & (1 << i) &&
            /* We also need to check for special memory properties pass to us */
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("VULKAN_DEVICE::FIND_MEMORY_TYPE::Failed to find suitable memory type");
}

VkCommandBuffer VulkanDevice::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphicsCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void VulkanDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, graphicsCommandPool, 1, &commandBuffer);
}