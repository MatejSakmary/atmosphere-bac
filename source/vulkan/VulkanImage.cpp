#include "VulkanImage.h"

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
    VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t depth)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    if(depth > 1)
    {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    } else {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    viewInfo.format = format;

    /* subresourceRnage describes what the image's purpose is and which part of the image
    should be accessed*/
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_IMAGE_VIEW::failed to create image view");
    }
    return imageView;
}

void VulkanImage::CreateImage(uint32_t width, uint32_t height, uint32_t depth,
    uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, 
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImageAspectFlags aspectFlags)
{
    VkImageType imageType = depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
    this->format = format;
    this->mipLevels = mipLevels;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;
    
    if (vkCreateImage(device->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_TEXTURE_IMAGE::Failed to create image");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->device, image, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(device->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_IMAGE::Failed to allocate image memory");
    }
    vkBindImageMemory(device->device, image, imageMemory, 0);
    
    imageView = createImageView(device->device, image, format, aspectFlags, mipLevels, depth);
}

VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, uint32_t width, uint32_t height,
    uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, 
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImageAspectFlags aspectFlags, uint32_t depth) : device{device}
{
    CreateImage(width, height, depth, mipLevels, numSamples, format,
        tiling, usage, properties, aspectFlags);
}


VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, const std::string &texturePath, bool isEXR) :
    device{device}  
{
    int texWidth, texHeight, texChannels;
    VkDeviceSize imageSize;
    void *imageData;
    if(isEXR)
    {
        float *rgba;
        const char* err;
        int ret = LoadEXR(&rgba, &texWidth, &texHeight, texturePath.c_str(), &err);
        imageSize = texWidth * texHeight * sizeof(float) * 4;
        if (ret != 0) 
        {
            std::cout << "err: " << std::string(err) << std::endl;
            throw std::runtime_error("VULKAN_IMAGE::CREATE_TEXTURE_IMAGE::Failed to load exr image");
        }
        imageData = rgba;
    } else {
        stbi_uc *rgba = stbi_load(texturePath.c_str(), &texWidth, &texHeight,
                                    &texChannels, STBI_rgb_alpha);

        imageSize = texWidth * texHeight * 4;
        if (!rgba)
        {
            throw std::runtime_error("VULKAN_IMAGE::CREATE_TEXTURE_IMAGE::Failed to load texture image");
        }
        imageData = rgba;
    }

    uint32_t mipLevels;
    /* Calculate the number of levels in the mip chain */
    if(isEXR)
    {
        mipLevels = 1;
    }else{
        mipLevels = static_cast<uint32_t>(
            std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    }

    /* TODO: This should be handled through the device */
    VulkanBuffer stagingBuffer = VulkanBuffer(device, imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;

    vkMapMemory(device->device, stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vkUnmapMemory(device->device, stagingBuffer.bufferMemory);
    if(isEXR)
    {
        free(imageData);
    }else{
        stbi_image_free(imageData);
    }

    VkFormat imageFormat;
    if(isEXR)
    {
        imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    }else{
        imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    }
    CreateImage(texWidth, texHeight, 1, mipLevels, 
                VK_SAMPLE_COUNT_1_BIT, imageFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

    TransitionImageLayout(imageFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    CopyBufferToImage(stagingBuffer,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
    /* transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps */
    if(!isEXR)
    {
        GenerateMipmaps(imageFormat, texWidth, texHeight, mipLevels);
    }
}

VulkanImage::~VulkanImage()
{
    vkDestroyImage(device->device, image, nullptr);
    vkDestroyImageView(device->device, imageView, nullptr);
    vkFreeMemory(device->device, imageMemory, nullptr);
}

void VulkanImage::TransitionImageLayout( VkFormat format,VkImageLayout oldLayout,
    VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = device->BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    /* Used when we wan't to use this barrier to transfer ownership (when using EXCLUSIVE_OWNERSHIP)*/
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    /* Change aspectMask to correctly represent image format */
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    /* specify specific part of image that is affected */
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        /* Transfer writes must occur in the pipeline transfer stage */
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        /* Since writes don't have to wait on anything, specify earliest
           possible pipelineStage for the pre-barrier operation */
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        /* NOTE: this is not a real stage within the graphics and compute
                 pipelines. It is more of a pseudo-stage where transfers happen */
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    else
    {
        throw std::invalid_argument("APP::TRANSITION_IMAGE_LAYOUT::Unsupported layout transition");
    }

    /* Barriers are primarily used for synchronization so I must specify which types of
       operations that involve the resource must happen before the barrier and which
       must wait for the barrier */
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    device->EndSingleTimeCommands(commandBuffer);
}

bool VulkanImage::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanImage::CopyBufferToImage(VulkanBuffer &buffer, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = device->BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};

    /* 4th parameter specifies which layout the image is currently using
        -> I'm assuming that the image has already been transitioned to the
           layout that is optimal for copying pixels to */
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer.buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    device->EndSingleTimeCommands(commandBuffer);
}

void VulkanImage::GenerateMipmaps( VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    /* Check if image format supports linear blitting */
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device->physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & 
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("APP::GENERATE_MIPMAPS::Texture image fromat does \
                                  not support linear blitting");
    }

    VkCommandBuffer commandBuffer = device->BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;


    for(uint32_t i = 1; i < mipLevels; i++)
    {
        /* first transition level i - 1 to TRANSFER SRC OPTIMAL
           This transition will wait for level i - 1 to be filled, either from the previous
           blit command, or from vkCmdCopyBufferToImage, current blit command will wait
           for this transition */
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,
                              mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        /* We are bliting from the image to itself just between different levels of the same image,
           Source was just transitioned earlier and destination is still set from createTextureImage */
        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            /* Use linear filter to eable interpolation */
            VK_FILTER_LINEAR);
        
        /* Transition mip level i - 1 to SHADER_READ_ONLY_OPTIMAL
           - waits on the current blit command to finish All sampling operations will
             wait on this transition to finish */
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    /* Transition the last mip level from DST_OPTIMAL to SHADER_READ_ONLY_OPTIMAL */
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    device->EndSingleTimeCommands(commandBuffer);
}