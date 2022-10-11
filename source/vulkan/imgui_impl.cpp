#include "imgui_impl.hpp"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        throw std::runtime_error("IMGUI_IMPL::ERROR:ABORT");
}


ImGuiImpl::ImGuiImpl(VkInstance instance, std::shared_ptr<VulkanDevice> &device, GLFWwindow* window, 
    std::unique_ptr<VulkanSwapChain> &swapchain) : showPostProcessWindow {true}
{

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(device->device, &pool_info, nullptr, &imguiDSPool);

    VkAttachmentDescription attachment = {};
    attachment.format = swapchain->swapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachment = {};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(device->device, &info, nullptr, &imguiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("IMGUI_IMPL::Failed to create imgui renderpass");
    } 

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = device->physicalDevice;
    initInfo.Device = device->device;
    initInfo.QueueFamily = device->familyIndices.graphicsFamily.value();
    initInfo.Queue = device->graphicsQueue;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = imguiDSPool;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = swapchain->minImageCount;
    initInfo.ImageCount = swapchain->imageCount;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&initInfo, imguiRenderPass);

    VkCommandBuffer commandBuffer = device->BeginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    device->EndSingleTimeCommands(commandBuffer);


    VkCommandPoolCreateInfo commandPoolCI = {};
    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCI.queueFamilyIndex = device->familyIndices.graphicsFamily.value();
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device->device, &commandPoolCI, nullptr, &imGuiCommandPool)
        != VK_SUCCESS)
    {
        throw std::runtime_error("IMGUI_IMPL::Failed to create imgui command pool");
    }

    imGuiCommandBuffers.resize(swapchain->imageCount);
    VkCommandBufferAllocateInfo commandBufferAI = {};
    commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAI.commandPool = imGuiCommandPool;
    commandBufferAI.commandBufferCount = swapchain->imageCount;
    if (vkAllocateCommandBuffers(device->device, &commandBufferAI, imGuiCommandBuffers.data())
        != VK_SUCCESS)
    {
        throw std::runtime_error("IMGUI_IMPL::Failed to allocate imgui command buffers");
    }
}

VkCommandBuffer ImGuiImpl::PrepareNewFrame(uint32_t imageIndex, VkFramebuffer framebuffer,
    Camera *camera, PostProcessParamsBuffer &postParams, AtmosphereParametersBuffer &atmoParams,
    CloudsParametersBuffer &cloudParams, glm::vec2 extent)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    glm::vec3 cameraPos = camera->getPos();
    ImGui::Text("Camera position is");
    ImGui::Text("x: %f", cameraPos.x);
    ImGui::SameLine();
    ImGui::Text("y: %f", cameraPos.y);
    ImGui::SameLine();
    ImGui::Text("z: %f", cameraPos.z);

    ImGui::Text("x: %f km", cameraPos.x * 0.01f);
    ImGui::SameLine();
    ImGui::Text("y: %f km", cameraPos.y * 0.01f);
    ImGui::SameLine();
    ImGui::Text("z: %f km", cameraPos.z * 0.01f);
    ImGui::Text("Yaw: %f   Pitch: %f", camera->getYaw(), camera->getPitch());

    if(ImGui::CollapsingHeader("CloudParameters"))
    {
        if(ImGui::TreeNode("Dimensions and scales"))
        {
            ImGui::SliderFloat("Cloud layer start", &cloudParams.minBounds ,0.0f, 100.0f);
            ImGui::SliderFloat("Cloud layer end", &cloudParams.maxBounds , 0.0f, 100.0f);
            ImGui::SliderFloat("Clouds scale", &cloudParams.cloudsScale , 0.0f, 5.0f);
            ImGui::SliderFloat("Detail scale", &cloudParams.detailScale, 0.0, 20.0);
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Noise Weights"))
        {
            if(ImGui::TreeNode("Base shape"))
            {
                ImGui::SliderFloat("Shape Noise Weight x", &cloudParams.shapeNoiseWeights.x, 0.0, 1.0);
                ImGui::SliderFloat("Shape Noise Weight y", &cloudParams.shapeNoiseWeights.y, 0.0, 1.0);
                ImGui::SliderFloat("Shape Noise Weight z", &cloudParams.shapeNoiseWeights.z, 0.0, 1.0);
                ImGui::SliderFloat("Shape Noise Weight w", &cloudParams.shapeNoiseWeights.w, 0.0, 1.0);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("Detail"))
            {
                ImGui::SliderFloat("Detail Noise Weight x", &cloudParams.detailNoiseWeights.x, 0.0, 1.0);
                ImGui::SliderFloat("Detail Noise Weight y", &cloudParams.detailNoiseWeights.y, 0.0, 1.0);
                ImGui::SliderFloat("Detail Noise Weight z", &cloudParams.detailNoiseWeights.z, 0.0, 1.0);
                ImGui::SliderFloat("Detail Noise Weight w", &cloudParams.detailNoiseWeights.w, 0.0, 1.0);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        ImGui::SliderFloat("Density offset", &cloudParams.densityOffset, 0.0, 3.0);
        ImGui::SliderFloat("Density multiplier", &cloudParams.densityMultiplier, 0.0, 3.0);
        ImGui::SliderFloat("Detail Noise multiplier", &cloudParams.detailNoiseMultiplier, 0.0, 3.0);
        ImGui::SliderInt("Clouds sample count", &cloudParams.sampleCount, 1, 200); 
        ImGui::SliderInt("To Sun sample count", &cloudParams.sampleCountToSun, 1, 100); 
        ImGui::SliderFloat("Abs to sun", &cloudParams.lightAbsTowardsSun, 0.0, 10.0); 
        ImGui::SliderFloat("Abs through cloud", &cloudParams.lightAbsThroughCloud, 0.0, 10.0); 
        ImGui::SliderFloat("Darkness threshold", &cloudParams.darknessThreshold, 0.0, 1.0); 
        ImGui::SliderInt("Debug", &cloudParams.debug, 0, 10); 
        ImGui::SliderFloat4("Phase parameters", glm::value_ptr(cloudParams.phaseParams), 0.0, 2.0);
    }
    if(ImGui::CollapsingHeader("Atmosphere Parameters"))
    {
        ImGui::SliderFloat("Sun Phi Angle", &atmoParams.sunPhiAngle, 0.0, 360.0); 
        ImGui::SliderFloat("Sun Theta Angle", &atmoParams.sunThetaAngle, 0.0, 360.0); 

        ImGui::SliderFloat("Bottom Radius", &atmoParams.bottom_radius, 1000.0, 8000.0);
        ImGui::SliderFloat("Top Radius", &atmoParams.top_radius, 1000.0, 8000.0);

        if(ImGui::TreeNode("Rayleigh Parameters"))
        {
            float rayScaleHeight = -1.0 / atmoParams.rayleigh_density[7];
            ImGui::SliderFloat("Rayleigh scale height", &rayScaleHeight, 0.1, 20.0);
            atmoParams.rayleigh_density[7] = -1.0 / rayScaleHeight;
            ImGui::SliderFloat3("Rayleigh scattering",glm::value_ptr(atmoParams.rayleigh_scattering),0.001, 0.1);
            ImGui::TreePop();
        }
        if(ImGui::TreeNode("Mie Parameters"))
        {
            float mieScaleHeight = -1.0 / atmoParams.mie_density[7];
            ImGui::SliderFloat("Mie scale height", &mieScaleHeight, 0.1, 20.0);
            atmoParams.mie_density[7] = -1.0 / mieScaleHeight;
            ImGui::SliderFloat3("Mie scattering", glm::value_ptr(atmoParams.mie_scattering),0.001, 0.1);
            ImGui::SliderFloat3("Mie extinction", glm::value_ptr(atmoParams.mie_extinction),0.001, 0.1);
            ImGui::TreePop();
        }
        ImGui::SliderFloat3("Ozone extinction", glm::value_ptr(atmoParams.absorption_extinction),0.0001, 0.1);

    }
    if(ImGui::CollapsingHeader("Post Process"))
    {
        ImGui::SliderFloat("minimum luminance", &postParams.minimumLuminance, 1.0f, 20000.0f); 
        ImGui::SliderFloat("maximum luminance", &postParams.maximumLuminance, 1.0f, 20000.0f); 
        ImGui::SliderFloat("Luminance adaptation rate", &postParams.lumAdaptTau, 1.0f, 2.0f, "%.3f"); 
        ImGui::SliderInt("Tonemap curve", &postParams.tonemapCurve, 1.0, 4.0); 
        switch(postParams.tonemapCurve)
        {
            case 1:
                ImGui::SliderFloat("whitepoint", &postParams.whitepoint, 0.1f, 10.0f); 
                break;
            case 2:
                break;
            case 3:
                ImGui::SliderFloat("max display brightness", &postParams.maxDisplayBrigtness, 1.0f, 100.0f); 
                ImGui::SliderFloat("contrast", &postParams.contrast, 0.0f, 5.0f); 
                ImGui::SliderFloat("linear section start", &postParams.linearSectionStart, 0.0f, 1.0f); 
                ImGui::SliderFloat("linear section length", &postParams.linearSectionLength, 0.0f, 1.0f); 
                ImGui::SliderFloat("black tightness", &postParams.black, 0.0f, 1.0f); 
                ImGui::SliderFloat("pedestal", &postParams.pedestal, 0.0f, 1.0f); 
                break;
            case 4:
                ImGui::SliderFloat("contrast", &postParams.a, 0.0f, 5.0f); 
                ImGui::SliderFloat("shoulder", &postParams.d, 0.0f, 5.0f); 
                ImGui::SliderFloat("hdrMax", &postParams.hdrMax, 0.0f, 16.0f); 
                ImGui::SliderFloat("midIn", &postParams.midIn, 0.0f, 2.0f); 
                ImGui::SliderFloat("midOut", &postParams.midOut, 0.0f, 2.0f); 
                break;
        }
    }

    /* Command buffer preparation */
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(imGuiCommandBuffers[imageIndex], &info);

    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderPassBeginInfo renderPassBegininfo = {};
    renderPassBegininfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegininfo.renderPass = imguiRenderPass;
    renderPassBegininfo.framebuffer = framebuffer;
    renderPassBegininfo.renderArea.extent.width = extent.x;
    renderPassBegininfo.renderArea.extent.height = extent.y;
    renderPassBegininfo.clearValueCount = 1;
    renderPassBegininfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(imGuiCommandBuffers[imageIndex], &renderPassBegininfo, VK_SUBPASS_CONTENTS_INLINE);
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imGuiCommandBuffers[imageIndex]);
    vkCmdEndRenderPass(imGuiCommandBuffers[imageIndex]);
    if (vkEndCommandBuffer(imGuiCommandBuffers[imageIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("IMGUI_IMPL::Failed to end command buffer");
    }
    return imGuiCommandBuffers[imageIndex];
}

