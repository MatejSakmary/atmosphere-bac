#include "worley_noise.hpp"
#define MAX_DIV_CNT 50
#define CHANNEL_CNT 4

WorleyNoise3D::WorleyNoise3D(glm::ivec3 texDimensions, WorleyNoiseCreateParams params, 
    std::shared_ptr<VulkanDevice> device, VkDescriptorPool pool) : 
    texDimensions{texDimensions}, params{params}, device{device}
{
    /* TODO: replace with randomly generated number after possibly */
    mt = std::mt19937(123);
    // mt = std::mt19937(12312341234);
    distribution = std::uniform_real_distribution<float>(0,1);

    noiseImage = std::make_unique<VulkanImage>(device, texDimensions.x,
        texDimensions.y, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, texDimensions.z);
    
    noiseImage->TransitionImageLayout(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL, 1);
    /* ====================== Channel independent Buffer Creation ================================ */
    VkDeviceSize maxBufferSize = MAX_DIV_CNT * MAX_DIV_CNT * MAX_DIV_CNT * sizeof(glm::vec3);

    for(int i = 0; i < CHANNEL_CNT; i++)
    {
        
        perChannelData[i].oneChannelNoiseImage = std::make_unique<VulkanImage>(device, texDimensions.x,
        texDimensions.y, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, texDimensions.z);

        perChannelData[i].oneChannelNoiseImage->TransitionImageLayout(VK_FORMAT_R16_SFLOAT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);

        perChannelData[i].pointsABuffer = std::make_unique<VulkanBuffer> (
            device, maxBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        perChannelData[i].pointsBBuffer = std::make_unique<VulkanBuffer> (
            device, maxBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        perChannelData[i].pointsCBuffer = std::make_unique<VulkanBuffer> (
            device, maxBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        perChannelData[i].minMaxBuffer = std::make_unique<VulkanBuffer> (
            device, sizeof(MinMaxParamsBufferObject), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 

        perChannelData[i].worleyParamsUBO = std::make_unique<VulkanBuffer> (
            device, sizeof(WorleyParamsBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    /* ====================== DS and DS Layout Creation ======================================= */
    #pragma region worleyDSCreation
    VkDescriptorSetLayoutBinding bufferADSLayoutBinding {};
    bufferADSLayoutBinding.binding = 0;
    bufferADSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferADSLayoutBinding.descriptorCount = 1;
    bufferADSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bufferADSLayoutBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding bufferBDSLayoutBinding {};
    bufferBDSLayoutBinding.binding = 1;
    bufferBDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferBDSLayoutBinding.descriptorCount = 1;
    bufferBDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bufferBDSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bufferCDSLayoutBinding {};
    bufferCDSLayoutBinding.binding = 2;
    bufferCDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferCDSLayoutBinding.descriptorCount = 1;
    bufferCDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bufferCDSLayoutBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding bufferMinMaxDSLayoutBinding {};
    bufferMinMaxDSLayoutBinding.binding = 3;
    bufferMinMaxDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bufferMinMaxDSLayoutBinding.descriptorCount = 1;
    bufferMinMaxDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bufferMinMaxDSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding paramsUBODSLayoutBinding {};
    paramsUBODSLayoutBinding.binding = 4;
    paramsUBODSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    paramsUBODSLayoutBinding.descriptorCount = 1;
    paramsUBODSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    paramsUBODSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding resultImageDSLayoutBinding; 
    resultImageDSLayoutBinding.binding = 5;
    resultImageDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    resultImageDSLayoutBinding.descriptorCount = 1;
    resultImageDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> worleyComputeBindings = {
        bufferADSLayoutBinding, bufferBDSLayoutBinding, bufferCDSLayoutBinding,
        paramsUBODSLayoutBinding, resultImageDSLayoutBinding, bufferMinMaxDSLayoutBinding 
    };

    VkDescriptorSetLayoutCreateInfo worleyComputeLayoutCI {};
    worleyComputeLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    worleyComputeLayoutCI.bindingCount = 6;
    worleyComputeLayoutCI.pBindings = worleyComputeBindings.data();

    for(int i = 0; i < CHANNEL_CNT; i++)
    {

        if (vkCreateDescriptorSetLayout(device->device, &worleyComputeLayoutCI,
            nullptr, &perChannelData[i].worleyNoiseDSLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("WORLEY_NOISE_3D::Failed to create DS Layout");
        }

        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &perChannelData[i].worleyNoiseDSLayout;

        if (vkAllocateDescriptorSets(device->device, &allocInfo, &perChannelData[i].worleyNoiseDS) != VK_SUCCESS)
        {
            throw std::runtime_error("WORLEY_NOISE_3D::Failed to create Descriptor set");
        }

        VkDescriptorBufferInfo bufferADSBufferInfo {};
        bufferADSBufferInfo.buffer = perChannelData[i].pointsABuffer->buffer;
        bufferADSBufferInfo.offset = 0;
        bufferADSBufferInfo.range = maxBufferSize;

        VkDescriptorBufferInfo bufferBDSBufferInfo {};
        bufferBDSBufferInfo.buffer = perChannelData[i].pointsBBuffer->buffer;
        bufferBDSBufferInfo.offset = 0;
        bufferBDSBufferInfo.range = maxBufferSize;

        VkDescriptorBufferInfo bufferCDSBufferInfo {};
        bufferCDSBufferInfo.buffer = perChannelData[i].pointsCBuffer->buffer;
        bufferCDSBufferInfo.offset = 0;
        bufferCDSBufferInfo.range = maxBufferSize;

        VkDescriptorBufferInfo bufferMinMaxDSBufferInfo {};
        bufferMinMaxDSBufferInfo.buffer = perChannelData[i].minMaxBuffer->buffer;
        bufferMinMaxDSBufferInfo.offset = 0;
        bufferMinMaxDSBufferInfo.range = sizeof(MinMaxParamsBufferObject);

        VkDescriptorBufferInfo paramsUBODSBufferInfo {};
        paramsUBODSBufferInfo.buffer = perChannelData[i].worleyParamsUBO->buffer;
        paramsUBODSBufferInfo.offset = 0;
        paramsUBODSBufferInfo.range = sizeof(WorleyParamsBufferObject);

        VkDescriptorImageInfo noiseImageDSInfo {};
        noiseImageDSInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        noiseImageDSInfo.imageView = perChannelData[i].oneChannelNoiseImage->imageView;

        std::array<VkWriteDescriptorSet, 6> updateDescriptorWrites{};
        updateDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[0].dstSet = perChannelData[i].worleyNoiseDS;
        updateDescriptorWrites[0].dstBinding = 0;
        updateDescriptorWrites[0].dstArrayElement = 0;
        updateDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        updateDescriptorWrites[0].descriptorCount = 1;
        updateDescriptorWrites[0].pBufferInfo = &bufferADSBufferInfo;

        updateDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[1].dstSet = perChannelData[i].worleyNoiseDS;
        updateDescriptorWrites[1].dstBinding = 1;
        updateDescriptorWrites[1].dstArrayElement = 0;
        updateDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        updateDescriptorWrites[1].descriptorCount = 1;
        updateDescriptorWrites[1].pBufferInfo = &bufferBDSBufferInfo;

        updateDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[2].dstSet = perChannelData[i].worleyNoiseDS;
        updateDescriptorWrites[2].dstBinding = 2;
        updateDescriptorWrites[2].dstArrayElement = 0;
        updateDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        updateDescriptorWrites[2].descriptorCount = 1;
        updateDescriptorWrites[2].pBufferInfo = &bufferCDSBufferInfo;

        updateDescriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[3].dstSet = perChannelData[i].worleyNoiseDS;
        updateDescriptorWrites[3].dstBinding = 3;
        updateDescriptorWrites[3].dstArrayElement = 0;
        updateDescriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        updateDescriptorWrites[3].descriptorCount = 1;
        updateDescriptorWrites[3].pBufferInfo = &bufferMinMaxDSBufferInfo;

        updateDescriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[4].dstSet = perChannelData[i].worleyNoiseDS;
        updateDescriptorWrites[4].dstBinding = 4;
        updateDescriptorWrites[4].dstArrayElement = 0;
        updateDescriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        updateDescriptorWrites[4].descriptorCount = 1;
        updateDescriptorWrites[4].pBufferInfo = &paramsUBODSBufferInfo;

        updateDescriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[5].dstSet = perChannelData[i].worleyNoiseDS;
        updateDescriptorWrites[5].dstBinding = 5;
        updateDescriptorWrites[5].dstArrayElement = 0;
        updateDescriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        updateDescriptorWrites[5].descriptorCount = 1;
        updateDescriptorWrites[5].pImageInfo = &noiseImageDSInfo;

        vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(updateDescriptorWrites.size()),
                               updateDescriptorWrites.data(), 0, nullptr);
    }

    #pragma endregion DSCreation

    #pragma region noiseDSCreation
    VkDescriptorSetLayoutBinding normalizeBuffersMinMaxDSLayoutBinding {};
    normalizeBuffersMinMaxDSLayoutBinding.binding = 0;
    normalizeBuffersMinMaxDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    normalizeBuffersMinMaxDSLayoutBinding.descriptorCount = 4;
    normalizeBuffersMinMaxDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    normalizeBuffersMinMaxDSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding channelResultImageDSLayoutBinding; 
    channelResultImageDSLayoutBinding.binding = 1;
    channelResultImageDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    channelResultImageDSLayoutBinding.descriptorCount = 4;
    channelResultImageDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding combinedChannelsNoiseImageDSLayoutBinding {};
    combinedChannelsNoiseImageDSLayoutBinding.binding = 2;
    combinedChannelsNoiseImageDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    combinedChannelsNoiseImageDSLayoutBinding.descriptorCount = 1;
    combinedChannelsNoiseImageDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> normalizeWorleyComputeBindings = {
        normalizeBuffersMinMaxDSLayoutBinding, channelResultImageDSLayoutBinding,
        combinedChannelsNoiseImageDSLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo normalizeWorleyComputeLayoutCI {};
    normalizeWorleyComputeLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    normalizeWorleyComputeLayoutCI.bindingCount = 3;
    normalizeWorleyComputeLayoutCI.pBindings = normalizeWorleyComputeBindings.data();

    if (vkCreateDescriptorSetLayout(device->device, &normalizeWorleyComputeLayoutCI,
        nullptr, &normalizeNoiseDSLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("WORLEY_NOISE_3D::Failed to create normalize noise DS Layout");
    }

    VkDescriptorSetAllocateInfo normalizeAllocInfo {};
    normalizeAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    normalizeAllocInfo.descriptorPool = pool;
    normalizeAllocInfo.descriptorSetCount = 1;
    normalizeAllocInfo.pSetLayouts = &normalizeNoiseDSLayout;
    
    if (vkAllocateDescriptorSets(device->device, &normalizeAllocInfo, &normalizeNoiseDS) != VK_SUCCESS)
    {
        throw std::runtime_error("WORLEY_NOISE_3D::Failed to create normalize noise Descriptor set");
    }

    std::array<VkDescriptorBufferInfo, 4> perChannelMinMaxBuffersInfo {};
    std::array<VkDescriptorImageInfo, 4> perChannelResultImagesInfo {};
    std::array<VkWriteDescriptorSet, 9> normalizeUpdateDS {};
    for (int i = 0; i < CHANNEL_CNT; i++)
    {
        perChannelMinMaxBuffersInfo[i].buffer = perChannelData[i].minMaxBuffer->buffer;
        perChannelMinMaxBuffersInfo[i].offset = 0;
        perChannelMinMaxBuffersInfo[i].range = sizeof(MinMaxParamsBufferObject);

        perChannelResultImagesInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        perChannelResultImagesInfo[i].imageView = perChannelData[i].oneChannelNoiseImage->imageView;

        normalizeUpdateDS[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalizeUpdateDS[i].dstSet = normalizeNoiseDS;
        normalizeUpdateDS[i].dstBinding = 0;
        normalizeUpdateDS[i].dstArrayElement = i;
        normalizeUpdateDS[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        normalizeUpdateDS[i].descriptorCount = 1;
        normalizeUpdateDS[i].pBufferInfo = &perChannelMinMaxBuffersInfo[i];

        normalizeUpdateDS[i + CHANNEL_CNT].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalizeUpdateDS[i + CHANNEL_CNT].dstSet = normalizeNoiseDS;
        normalizeUpdateDS[i + CHANNEL_CNT].dstBinding = 1;
        normalizeUpdateDS[i + CHANNEL_CNT].dstArrayElement = i;
        normalizeUpdateDS[i + CHANNEL_CNT].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        normalizeUpdateDS[i + CHANNEL_CNT].descriptorCount = 1;
        normalizeUpdateDS[i + CHANNEL_CNT].pImageInfo = &perChannelResultImagesInfo[i];
    }

    VkDescriptorImageInfo finalImageDSInfo {};
    finalImageDSInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    finalImageDSInfo.imageView = noiseImage->imageView;

    normalizeUpdateDS[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    normalizeUpdateDS[8].dstSet = normalizeNoiseDS;
    normalizeUpdateDS[8].dstBinding = 2;
    normalizeUpdateDS[8].dstArrayElement = 0;
    normalizeUpdateDS[8].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    normalizeUpdateDS[8].descriptorCount = 1;
    normalizeUpdateDS[8].pImageInfo = &finalImageDSInfo;

    vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(normalizeUpdateDS.size()),
                           normalizeUpdateDS.data(), 0, nullptr);
    #pragma end noiseDSCreation

    /* ===================== Create Vulkan pipelines ========================================== */
    auto worleyNoiseComputeShaderCode = readFile("shaders/build/noise/worley_noise_3D.spv");
    VkShaderModule worleyNoiseComputeShaderModule = 
        createShaderModule(device, worleyNoiseComputeShaderCode);

    worleyNoisePipeline = std::make_unique<VulkanPipeline>(
        device,
        VulkanPipeline::initPiplineLayoutCI(perChannelData[0].worleyNoiseDSLayout),
        VulkanPipeline::initComputeShaderStageCI(worleyNoiseComputeShaderModule)
    );
     
    vkDestroyShaderModule(device->device, worleyNoiseComputeShaderModule, nullptr);


    auto normalizeNoiseComputeShaderCode = readFile("shaders/build/noise/normalize_noise_3D.spv");
    VkShaderModule normalizeNoiseComputeShaderModule = 
        createShaderModule(device, normalizeNoiseComputeShaderCode);
    normalizeNoisePipeline = std::make_unique<VulkanPipeline>(
        device,
        VulkanPipeline::initPiplineLayoutCI(normalizeNoiseDSLayout),
        VulkanPipeline::initComputeShaderStageCI(normalizeNoiseComputeShaderModule)
    );
    worleyNoiseCommadBuffer = device->createComputeCommandBuffer();
    vkDestroyShaderModule(device->device, normalizeNoiseComputeShaderModule, nullptr);

    /* ===================== Create and record command buffer ==================================== */
    VkCommandBufferBeginInfo beginWorleyCmdBuffer {};
    beginWorleyCmdBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if(vkBeginCommandBuffer(worleyNoiseCommadBuffer, &beginWorleyCmdBuffer) 
        != VK_SUCCESS)
    {
        throw std::runtime_error("WORLEY NOISE 3D::Failed to begin command buffer");
    }

    vkCmdBindPipeline(worleyNoiseCommadBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
        worleyNoisePipeline->pipeline);
    for (int i = 0; i < CHANNEL_CNT; i++)
    {
        vkCmdBindDescriptorSets(worleyNoiseCommadBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            worleyNoisePipeline->layout, 0, 1, &perChannelData[i].worleyNoiseDS, 0, nullptr);
        vkCmdDispatch(worleyNoiseCommadBuffer, texDimensions.x / 4, texDimensions.y /4 , texDimensions.z / 4);
    }

    VkMemoryBarrier prevComputeWorkFinished = {};
    prevComputeWorkFinished.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    prevComputeWorkFinished.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    prevComputeWorkFinished.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        worleyNoiseCommadBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0, 1,
        &prevComputeWorkFinished, 
        0, nullptr,
        0, nullptr
    );

    vkCmdBindPipeline(worleyNoiseCommadBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        normalizeNoisePipeline->pipeline);
    vkCmdBindDescriptorSets(worleyNoiseCommadBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
        normalizeNoisePipeline->layout, 0, 1, &normalizeNoiseDS, 0, nullptr);
    vkCmdDispatch(worleyNoiseCommadBuffer, texDimensions.x / 4, texDimensions.y /4 , texDimensions.z / 4);
    vkEndCommandBuffer(worleyNoiseCommadBuffer);

    /* ====================== Filling buffers with points ======================================= */

    std::array<glm::vec3, 4> numDivisionsChannels = {
        params.numDivisionsRChannel,
        params.numDivisionsGChannel,
        params.numDivisionsBChannel,
        params.numDivisionsAChannel,
    };

    std::array<float, 4> persistenceChannels = {
        params.persistenceRChannel, 
        params.persistenceGChannel, 
        params.persistenceBChannel, 
        params.persistenceAChannel
    };

    for (int i = 0; i < CHANNEL_CNT; i++)
    {
        generateWorleyPointsBuffer(perChannelData[i].pointsABufferCPU, numDivisionsChannels[i].x);
        generateWorleyPointsBuffer(perChannelData[i].pointsBBufferCPU, numDivisionsChannels[i].y);
        generateWorleyPointsBuffer(perChannelData[i].pointsCBufferCPU, numDivisionsChannels[i].z);

        MinMaxParamsBufferObject minMaxCPUBuffer {10000000, 0};
        copyCPUBufferIntoGPUBuffer(perChannelData[i].pointsABufferCPU.data(), perChannelData[i].pointsABuffer, 
            glm::pow(numDivisionsChannels[i].x, 3) * sizeof(glm::vec3));
        copyCPUBufferIntoGPUBuffer(perChannelData[i].pointsBBufferCPU.data(), perChannelData[i].pointsBBuffer,
            glm::pow(numDivisionsChannels[i].y, 3) * sizeof(glm::vec3));
        copyCPUBufferIntoGPUBuffer(perChannelData[i].pointsCBufferCPU.data(), perChannelData[i].pointsCBuffer,
            glm::pow(numDivisionsChannels[i].z, 3) * sizeof(glm::vec3));
        copyCPUBufferIntoGPUBuffer(&minMaxCPUBuffer, perChannelData[i].minMaxBuffer, sizeof(MinMaxParamsBufferObject));

        perChannelData[i].worleyParams.numDivisions = numDivisionsChannels[i];
        perChannelData[i].worleyParams.texDimensions = texDimensions;
        perChannelData[i].worleyParams.targetChannels = glm::vec4(1.0, 0.0, 0.0, 0.0);
        perChannelData[i].worleyParams.persistence = persistenceChannels[i];

        void* data;
        vkMapMemory(device->device, perChannelData[i].worleyParamsUBO->bufferMemory, 0, sizeof(WorleyParamsBufferObject), 0, &data);
        memcpy(data, &perChannelData[i].worleyParams, sizeof(WorleyParamsBufferObject));
        vkUnmapMemory(device->device, perChannelData[i].worleyParamsUBO->bufferMemory);
    }

}

void WorleyNoise3D::copyCPUBufferIntoGPUBuffer(void* cpuData,
    std::unique_ptr<VulkanBuffer> &GPUBuffer, VkDeviceSize bufferSize)
{
    VulkanBuffer stagingBuffer = VulkanBuffer(device, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); 

    void* data;
    std::vector<float> data__;
    vkMapMemory(device->device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, cpuData, (size_t)bufferSize);
    vkUnmapMemory(device->device, stagingBuffer.bufferMemory);

    GPUBuffer->CopyIntoBuffer(stagingBuffer, bufferSize);
}

void WorleyNoise3D::generateWorleyPointsBuffer(std::vector<glm::vec3> &buffer, int numDivisions)
{
    buffer.resize(numDivisions * numDivisions * numDivisions);
    float cellSize = 1.0f / numDivisions;

    for (int x = 0; x < numDivisions; x++)
    {
        for (int y = 0; y < numDivisions; y++)
        {
            for (int z = 0; z < numDivisions; z++)
            {
                glm::vec3 randomOffset = glm::vec3(getRandNum(), getRandNum(), getRandNum()) * cellSize;
                glm::vec3 cellCorner = glm::vec3(x, y, z) * cellSize;

                int index = x + numDivisions * ( y + z * numDivisions );
                buffer[index] = cellCorner + randomOffset;
            }
        }
    }
}

void WorleyNoise3D::generateNoise()
{
    VkPipelineStageFlags waitFlags[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &worleyNoiseCommadBuffer;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitFlags;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    if (vkQueueSubmit(device->computeQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::DRAW_FRAME::Failed to submit draw command buffer");
    }
}

float WorleyNoise3D::getRandNum() { return distribution(mt); }

WorleyNoise3D::~WorleyNoise3D()
{
    for(auto& perChannel : perChannelData)
    {
        vkDestroyDescriptorSetLayout(device->device, perChannel.worleyNoiseDSLayout, nullptr);
    }
    vkDestroyDescriptorSetLayout(device->device, normalizeNoiseDSLayout, nullptr);
}

