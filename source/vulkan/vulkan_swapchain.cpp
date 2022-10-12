#include "vulkan_swapchain.hpp"
#include <iostream>
#include <algorithm>

VulkanSwapChain::VulkanSwapChain(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface,
    int width, int height) : device{device}, surface{surface}
{
    /* TODO: Move this to device */
    SwapChainSupportDetails details = device->querySwapChainSupport(device->physicalDevice, surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
    VkExtent2D extent = chooseSwapExtent(details.capabilities, width, height);

    /* It is recommended to request at least one more than minimum so we don't have to wait
       for driver to complete internal operations before aquiring another image */
    minImageCount = details.capabilities.minImageCount;
    imageCount = details.capabilities.minImageCount + 1;

    /* Make sure not to exceed the maximum
        NOTE: 0 is special value for "there is no maximum"*/
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount)
    {
        imageCount = details.capabilities.maxImageCount;
    }
    std::cout << "VULKAN_SWAP_CHAIN::Choosing swap chain image count: " << imageCount << std::endl;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    /* specifies the amount of layers image consist of */
    createInfo.imageArrayLayers = 1;
    /* specify that we want to render directly into the image -> used as color attachment*/
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /* specify how to handle swap chain images used across multiple queue families*/
    uint32_t queueFamilyIndices[] = {device->familyIndices.graphicsFamily.value(),
                                     device->familyIndices.presentFamily.value()};

    if (device->familyIndices.graphicsFamily != device->familyIndices.presentFamily)
    {
        /* image can be used across multiple queue families without
           explicit ownership transfers */
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        /* image in swap chain is is owned by one family and must be
           explicitly transfered before using it in another family */
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // OPTIONAL
        createInfo.pQueueFamilyIndices = nullptr;
    }

    /* transform applied to image -> 90° rotation or horizontal flip*/
    createInfo.preTransform = details.capabilities.currentTransform;
    /* specifies if alpha channel should be used for blending with other windows in the system
        -> in most cases we want to simply ignore*/
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    /* We don't care about pixels that are obscured f.e. by another window*/
    createInfo.clipped = VK_TRUE;
    /* When the window f.e. gets resized we need to create new swap chain
       and point it to the old one*/
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device->device, &createInfo, nullptr, &swapChain);
    if (result != VK_SUCCESS)
    {
        std::cout << "Result is " << result << std::endl;
        throw std::runtime_error("VULKAN_SWAP_CHAIN::CREATE_SWAPCHAIN::Failed to create swapchain");
    }

    /* Previously we only specified min number of images, there could be more*/
    vkGetSwapchainImagesKHR(device->device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->device, swapChain, &imageCount, swapChainImages.data());
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = createImageView(device->device, swapChainImages[i],
            swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
    }
}

VulkanSwapChain::~VulkanSwapChain()
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device->device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device->device, swapChain, nullptr);
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            std::cout << "Found B8G8R8A8 SRGB NONLINEAR format" << std::endl;
            return availableFormat;
        }
    }
    std::cout << "Failed to find wanted Swap Surface format selecting default" << std::endl;
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        /* Find if mailbox present mode is supported */
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::cout << "Found Mailbox present mode" << std::endl;
            return availablePresentMode;
        }
        // else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        // {
        //     std::cout << "Found Immediate present mode" << std::endl;
        //     return availablePresentMode;
        // }
    }

    /* Falling back to FIFO which is guaranteed to be supported */
    std::cout << "Mailbox not found falling back to FIFO" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities, int width, int height)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        std::cout << "Matching Swap extent to the resolution of the window "
                    << capabilities.currentExtent.width << " " << capabilities.currentExtent.height
                    << std::endl;

        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        /* clamp to fit into supported dimensions */
        actualExtent.width = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

