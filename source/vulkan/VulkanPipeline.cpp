#include "VulkanPipeline.h"

std::vector<char> readFile(const std::string &filename)
{
    /* ate -> start reading at the end of the file */
    /* binary -> read the file as binary*/
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file " + filename);
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule createShaderModule(std::shared_ptr<VulkanDevice> device, const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    /* bytecode is a uint32 pointer not char pointer
       NOTE: when casting like this we need to make sure to satisfy
             the allignment requirements of uint32 -> vector does this
             for us */
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_SHADER_MODULE::failed to create shader module");
    }
    return shaderModule;
}

#pragma region initializers
VkPipelineShaderStageCreateInfo VulkanPipeline::initVertexShaderStageCI(
    VkShaderModule module)
{
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = module;
    /* specify entry point*/
    vertexShaderStageInfo.pName = "main";
    /* Used to initialize some constants in our shader -> used to alter
       behavior at pipeline creation so compiler can still perform some
       optimalizations */
    vertexShaderStageInfo.pSpecializationInfo = nullptr;

    return vertexShaderStageInfo;
}

VkPipelineShaderStageCreateInfo VulkanPipeline::initFragmentShaderStageCI(
    VkShaderModule module)
{
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = module;
    fragmentShaderStageInfo.pName = "main";
    fragmentShaderStageInfo.pSpecializationInfo = nullptr;

    return fragmentShaderStageInfo;
}

VkPipelineShaderStageCreateInfo VulkanPipeline::initComputeShaderStageCI(
    VkShaderModule module)
{
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = module;
    computeShaderStageInfo.pName = "main";
    computeShaderStageInfo.pSpecializationInfo = nullptr;

    return computeShaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::initVertexStageInputStateCI(
    const std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo VulkanPipeline::initInputAssemblyStateCI(
    VkPrimitiveTopology topology, 
    VkBool32 primitiveRestartEnable)
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = topology;
    /* If this is set to true, it is possible to break up lines and triangles in
       _STRIP topology by using a special index */
    inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
    return inputAssembly;
}


VkViewport VulkanPipeline::initViewport(
    float x, float y,
    float width, float height,
    float mindepth, float maxdepth)
{
    VkViewport viewport {};
    viewport.x = x;
    viewport.y = y;
    /* Size of the swap chain and images might differ from the WIDTH and HEIGHT of the
       window. The swap chain images will be used as framebuffers so use their size*/
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = mindepth;
    viewport.maxDepth = maxdepth;

    return viewport;
}

VkPipelineViewportStateCreateInfo VulkanPipeline::initViewportStateCI(
    bool dynamicState,
    const VkViewport &viewport,
    const VkRect2D &scissor)
{
    if(dynamicState)
    {
        throw std::runtime_error("APP::GRAPHICS_PIPELINE::INIT_VIEWPORT_STATE::Dynamic state\
            not supported");
    }

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    return viewportState;
}

VkPipelineViewportStateCreateInfo VulkanPipeline::initViewportStateCI(
    bool dynamicState,
    uint32_t viewportCount,
    uint32_t scissorCount,
    const std::vector<VkViewport> &viewports,
    const std::vector<VkRect2D> &scissors)
{
    if(dynamicState)
    {
        throw std::runtime_error("APP::GRAPHICS_PIPELINE::INIT_VIEWPORT_STATE::Dynamic state\
            not supported");
    }

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = static_cast<uint32_t>(viewports.size());
    viewportState.pViewports = viewports.data();
    viewportState.scissorCount = static_cast<uint32_t>(scissors.size());
    viewportState.pScissors = scissors.data();

    return viewportState;
}

VkPipelineRasterizationStateCreateInfo VulkanPipeline::initRaserizationStateCI(
    VkPolygonMode polygonMode,
    VkCullModeFlags cullMode,
    VkFrontFace frontFace,
    VkBool32 depthClampEnable,
    VkBool32 rasterizerDiscardEnable,
    float lineWidth)
{
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    /* If this is set to true fragments that are beyong the near and far plaens are
       clamped to them instead of discarding them */
    rasterizer.depthClampEnable = depthClampEnable;
    /* If this is true geometry never passes through the rasterizer stage*/
    rasterizer.rasterizerDiscardEnable = rasterizerDiscardEnable;
    rasterizer.polygonMode = polygonMode;
    rasterizer.lineWidth = lineWidth;
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;
    /* rasterizer can alter depth values by addig a constant value or biasing them
       based on fragment's slope*/
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VulkanPipeline::initMultisampleStateCI(
    VkBool32 enableSampleShading,
    float minSampleShading,
    VkSampleCountFlagBits rasterizationSamples)
{
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    /* Enable sample shading in the pipeline */
    multisampling.sampleShadingEnable = enableSampleShading;
    /* Min sample shading for sample shading, closer is smoother */
    multisampling.minSampleShading = minSampleShading;
    multisampling.rasterizationSamples = rasterizationSamples;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;

    return multisampling;
}

VkPipelineDepthStencilStateCreateInfo VulkanPipeline::initDepthStencilStateCI(
    VkBool32 depthTestEnable,
    VkBool32 depthWriteEnable,
    VkCompareOp depthCompareOp,
    VkBool32 stencilTestEnable)
{
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = depthTestEnable;
    depthStencil.depthWriteEnable = depthWriteEnable;
    depthStencil.depthCompareOp = depthCompareOp;
    /* Used for optional depth bounds tests - allows me to keep only the values
       which fall between the specified range*/
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // OPTIONAL
    depthStencil.maxDepthBounds = 1.0f; // OPTIONAL

    depthStencil.stencilTestEnable = stencilTestEnable;
    depthStencil.front = {}; // OPTIONAL
    depthStencil.back = {};  // OPTIONAL

    return depthStencil;
}

VkPipelineColorBlendAttachmentState VulkanPipeline::initColorBlendAttachmentAlpha0ignore(
    VkColorComponentFlags colorWriteMask,
    VkBool32 blendEnable)
{
    /* Contains configuration per attached framebuffer */
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = colorWriteMask;
    colorBlendAttachment.blendEnable = blendEnable;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; 
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;            
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;            

    return colorBlendAttachment;
}

VkPipelineColorBlendAttachmentState VulkanPipeline::initColorBlendAttachmentSrcAlphaDst(
    VkColorComponentFlags colorWriteMask,
    VkBool32 blendEnable)
{
    /* Contains configuration per attached framebuffer */
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = colorWriteMask;
    colorBlendAttachment.blendEnable = blendEnable;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; 
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;            
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;            

    return colorBlendAttachment;
}


VkPipelineColorBlendAttachmentState VulkanPipeline::initColorBlendAttachment(
    VkColorComponentFlags colorWriteMask,
    VkBool32 blendEnable)
{
    /* Contains configuration per attached framebuffer */
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = colorWriteMask;
    colorBlendAttachment.blendEnable = blendEnable;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; 
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE; 
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;            
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;            

    return colorBlendAttachment;
}


VkPipelineColorBlendStateCreateInfo VulkanPipeline::initColorBlendStateCI(
    const VkPipelineColorBlendAttachmentState &pAttachment)
{
    /* Contains global color blending settings*/
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    /* If true use bitwise blending note that this will automatically disable the
       first method (blend attachment) for every attached framebuffer*/
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // OPTIONAL
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &pAttachment;
    colorBlending.blendConstants[0] = 0.0f; // OPTIONAL
    colorBlending.blendConstants[1] = 0.0f; // OPTIONAL
    colorBlending.blendConstants[2] = 0.0f; // OPTIONAL
    colorBlending.blendConstants[3] = 0.0f; // OPTIONAL

    return colorBlending;
}

VkPipelineColorBlendStateCreateInfo VulkanPipeline::initColorBlendStateCI(
    uint32_t attachmentCount,
    const std::vector<VkPipelineColorBlendAttachmentState> &pAttachments)
{
    /* Contains global color blending settings*/
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    /* If true use bitwise blending note that this will automatically disable the
       first method (blend attachment) for every attached framebuffer*/
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // OPTIONAL
    colorBlending.attachmentCount = attachmentCount;
    colorBlending.pAttachments = pAttachments.data();
    colorBlending.blendConstants[0] = 0.0f; // OPTIONAL
    colorBlending.blendConstants[1] = 0.0f; // OPTIONAL
    colorBlending.blendConstants[2] = 0.0f; // OPTIONAL
    colorBlending.blendConstants[3] = 0.0f; // OPTIONAL

    return colorBlending;
}


VkPipelineLayoutCreateInfo VulkanPipeline::initPiplineLayoutCI(
    const VkDescriptorSetLayout &pSetLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;   
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    return pipelineLayoutInfo;
}

VkPipelineLayoutCreateInfo VulkanPipeline::initPiplineLayoutCI(
    uint32_t setLayoutCount,
    const std::vector<VkDescriptorSetLayout> &pSetLayouts)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayoutCount;
    pipelineLayoutInfo.pSetLayouts = pSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // OPTIONAL
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // OPTIONAL

    return pipelineLayoutInfo;
}

#pragma endregion initializers

VulkanPipeline::VulkanPipeline(
    std::shared_ptr<VulkanDevice> device,
    const VkPipelineLayoutCreateInfo &layoutCreateInfo,
    const VkPipelineShaderStageCreateInfo &shaderStage) : device{device}
{
    if (vkCreatePipelineLayout(device->device, &layoutCreateInfo,
        nullptr, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_COMPUTE_PIPELINE::\
                                  failed to create compute pipeline layout");
    }

    VkComputePipelineCreateInfo computePipelineCI{};
    computePipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCI.layout = layout;
    computePipelineCI.flags = 0;
    computePipelineCI.stage = shaderStage;

    if (vkCreateComputePipelines(device->device, VK_NULL_HANDLE, 1, &computePipelineCI,
        nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::COMPUTE_PIPELINE::\
        Failed to create compute pipeline");
    }
}



VulkanPipeline::VulkanPipeline(
    std::shared_ptr<VulkanDevice> device,
    uint32_t stageCount,
    const std::vector<VkPipelineShaderStageCreateInfo> pShaderStages,
    const VkPipelineVertexInputStateCreateInfo &vertexInputStage,
    const VkPipelineInputAssemblyStateCreateInfo &inputAssemblyState,
    const VkPipelineViewportStateCreateInfo &viewportState,
    const VkPipelineRasterizationStateCreateInfo &rasterizationState,
    const VkPipelineMultisampleStateCreateInfo &multisampleState,
    const VkPipelineDepthStencilStateCreateInfo &depthStencilState,
    const VkPipelineColorBlendStateCreateInfo &colorBlendState,
    const VkPipelineLayoutCreateInfo &layoutCreateInfo,
    const VkRenderPass renderPass,
    const uint32_t subpass) : device{device}
{
    if (vkCreatePipelineLayout(device->device, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_GRAPHICS_PIPELINE::failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = stageCount;
    pipelineInfo.pStages = pShaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputStage;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    /* Vulkan allows you to create a new graphics pipeline by deriving from an existing one*/
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // OPTIONAL
    pipelineInfo.basePipelineIndex = -1;              // OPTIONAL

    if (vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("PIPELINE::CONSTRUCTOR:Failed to create graphics pipeline");
    }
}

VulkanPipeline::~VulkanPipeline()
{
    vkDestroyPipeline(device->device, pipeline, nullptr);
    vkDestroyPipelineLayout(device->device, layout, nullptr);
}