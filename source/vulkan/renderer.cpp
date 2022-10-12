#include <cstring>
#include <iostream>
#include <iomanip>

#include "renderer.hpp"

template<typename T>
T findInMap(std::unordered_map<std::string, T> map, const std::string hash)
{
    auto elem = map.find(hash);
    if(elem == map.end())
    {
        throw std::runtime_error("Value " + hash + " not found in umap");
    } else 
    {
        return elem->second;
    }
}

void Renderer::createPreset(int presetNum)
{
    if(presetNum == 1)
    {
        cloudsParamsBuffer = CloudsParametersBuffer{};
        cloudsParamsBuffer.minBounds = 5.720;
        cloudsParamsBuffer.maxBounds = 8.0;
        cloudsParamsBuffer.cloudsScale = 1.386;
        cloudsParamsBuffer.detailScale = 8.911;
        cloudsParamsBuffer.shapeNoiseWeights = glm::vec4(0.559, 0.272, 0.125, 0.158);
        cloudsParamsBuffer.detailNoiseWeights = glm::vec4(0.767, 0.262, 0.168, 0.277);
        cloudsParamsBuffer.densityOffset = 0.817;
        cloudsParamsBuffer.densityMultiplier = 1.069;
        cloudsParamsBuffer.detailNoiseMultiplier = 0.329;
        cloudsParamsBuffer.sampleCount = 60;
        cloudsParamsBuffer.sampleCountToSun = 5;
        cloudsParamsBuffer.lightAbsTowardsSun = 0.248;
        cloudsParamsBuffer.lightAbsThroughCloud = 0.446;
        cloudsParamsBuffer.darknessThreshold = 0.238;
        cloudsParamsBuffer.debug = 1;
        cloudsParamsBuffer.phaseParams = glm::vec4(0.863, -0.528, 1.676, 0.216);

        SetupAtmosphereParametersBuffer(atmoParamsBuffer);

        postProcessParamsBuffer = PostProcessParamsBuffer{};
        postProcessParamsBuffer.minimumLuminance = 100.0;
        postProcessParamsBuffer.maximumLuminance = 6000.0;
        postProcessParamsBuffer.lumAdaptTau = 1.1;
        postProcessParamsBuffer.tonemapCurve = 4;
        postProcessParamsBuffer.whitepoint = 4.0;
        postProcessParamsBuffer.maxDisplayBrigtness = 1.0;
        postProcessParamsBuffer.contrast = 1.0;
        postProcessParamsBuffer.linearSectionLength = 0.22;
        postProcessParamsBuffer.linearSectionLength = 0.4;
        postProcessParamsBuffer.black = 1.33;
        postProcessParamsBuffer.pedestal = 0.0;
        postProcessParamsBuffer.a = 1.6;
        postProcessParamsBuffer.d = 0.977;
        postProcessParamsBuffer.hdrMax = 8.0;
        postProcessParamsBuffer.midIn = 0.18;
        postProcessParamsBuffer.midOut = 0.267;
    }
    if(presetNum == 2)
    {
        cloudsParamsBuffer = CloudsParametersBuffer{};
        cloudsParamsBuffer.minBounds = 3.808;
        cloudsParamsBuffer.maxBounds = 25.804;
        cloudsParamsBuffer.cloudsScale = 0.632;
        cloudsParamsBuffer.detailScale = 6.149;
        cloudsParamsBuffer.shapeNoiseWeights = glm::vec4(0.646, 0.272, 0.125, 0.000);
        cloudsParamsBuffer.detailNoiseWeights = glm::vec4(0.625, 0.272, 0.164, 0.328);
        cloudsParamsBuffer.densityOffset = 0.831;
        cloudsParamsBuffer.densityMultiplier = 0.639;
        cloudsParamsBuffer.detailNoiseMultiplier = 0.329;
        cloudsParamsBuffer.sampleCount = 120;
        cloudsParamsBuffer.sampleCountToSun = 4;
        cloudsParamsBuffer.lightAbsTowardsSun = 0.574;
        cloudsParamsBuffer.lightAbsThroughCloud = 0.101;
        cloudsParamsBuffer.darknessThreshold = 0.181;
        cloudsParamsBuffer.debug = 1;
        cloudsParamsBuffer.phaseParams = glm::vec4(0.857, -0.528, 2.0, 0.226);

        SetupAtmosphereParametersBuffer(atmoParamsBuffer);
        atmoParamsBuffer.rayleigh_density[7] = -1.0/18.416;
        atmoParamsBuffer.rayleigh_scattering = glm::vec3(0.149, 0.142, 0.051);
        atmoParamsBuffer.mie_density[7] = -1.0/13.143;
        atmoParamsBuffer.mie_scattering = glm::vec3(0.046, 0.047, 0.057);
        atmoParamsBuffer.mie_extinction = glm::vec3(0.082, 0.071, 0.058);

        postProcessParamsBuffer = PostProcessParamsBuffer{};
        postProcessParamsBuffer.minimumLuminance = 100.0;
        postProcessParamsBuffer.maximumLuminance = 6000.0;
        postProcessParamsBuffer.lumAdaptTau = 1.1;
        postProcessParamsBuffer.tonemapCurve = 4;
        postProcessParamsBuffer.whitepoint = 4.0;
        postProcessParamsBuffer.maxDisplayBrigtness = 1.0;
        postProcessParamsBuffer.contrast = 1.0;
        postProcessParamsBuffer.linearSectionStart = 0.22;
        postProcessParamsBuffer.linearSectionLength = 0.4;
        postProcessParamsBuffer.black = 1.33;
        postProcessParamsBuffer.pedestal = 0.0;
        postProcessParamsBuffer.a = 1.890;
        postProcessParamsBuffer.d = 1.070;
        postProcessParamsBuffer.hdrMax = 8.0;
        postProcessParamsBuffer.midIn = 0.314;
        postProcessParamsBuffer.midOut = 0.522;
    }
}

Renderer::Renderer(GLFWwindow *window, bool enableValidation) : framebufferResized{false}, validationEnabled{enableValidation}
{
    perFrameData = {};
    this->window = window;
    postProcessParamsBuffer = PostProcessParamsBuffer{
        100.0, 6000.0, 0.0, 1.1,
        4.0,                             // Reinhard
        1.0, 1.0, 0.22, 0.4, 1.33, 0.0,  // Uchimura
        1.6, 0.977, 8.0, 0.18, 0.267,    // Lottes
        4, glm::vec2(0.0, 0.0)
    };

    cloudsParamsBuffer = CloudsParametersBuffer{
        glm::vec4( 0.854, 0.130, 0.076, 0.515),
        glm::vec4( 0.0, 1.0, 0.5, 0.5),
        // glm::vec4( 0.7, 0.4, 0.4, 0.2),
        0.329, 
        4.0, 8.306, 0.3, 6.645,
        0.688, 0.269,
        100, 4,
        3.123, 0.100, 0.093, 1,
        glm::vec4(0.52, 0.52, 0.700, 0.100)
    };
    SetupAtmosphereParametersBuffer(atmoParamsBuffer);
    /* TODO: Renderer shouldn't own camera */
    camera = new Camera(
        glm::vec3(-240.0f, 66.0f, 11.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    createPreset(1);
    /* TODO: This really shouldn't be hardcoded like this */
    // createInstance(true);
    createInstance(enableValidation);
    createSurface();
    if(enableValidation) {setupDebugMessenger(instance, &debugMessenger);}
    vDevice = std::make_shared<VulkanDevice>(instance, surface);
    vDevice->createCommandPool();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    vSwapChain = std::make_unique<VulkanSwapChain>(vDevice, surface, width, height);
    postProcessParamsBuffer.texDimensions = 
        glm::vec2(vSwapChain->swapChainExtent.width, vSwapChain->swapChainExtent.height);
    /* TODO: change hacky unique ptr and shared ptr access */
    imguiImpl = std::make_unique<ImGuiImpl>(instance, vDevice, window, vSwapChain);

    loadAssets();
    createAttachments();
    createRenderPass();
    createDescriptorSetLayout();

    createPipelines();

    prepareTextureTargets(256, 64, VK_FORMAT_R16G16B16A16_SFLOAT);

    createQuerryPool();

    createFramebuffers();
    createPrimitivesBuffers();
    createUniformBuffers();
    createDescriptorPool();

    WorleyNoiseCreateParams worleyParams {};
    worleyParams.numDivisionsRChannel = glm::vec3(3, 8, 18);
    worleyParams.numDivisionsGChannel = glm::vec3(7, 15, 28);
    worleyParams.numDivisionsBChannel = glm::vec3(9, 18, 31);
    worleyParams.numDivisionsAChannel = glm::vec3(14, 22, 43);
    worleyParams.persistenceRChannel = 0.75f;
    worleyParams.persistenceGChannel = 0.95f;
    worleyParams.persistenceBChannel = 0.85f;
    worleyParams.persistenceAChannel = 0.85f;
    noise = std::make_unique<WorleyNoise3D>(glm::vec3(256, 256, 256), worleyParams,
        vDevice, descriptorPool);

    WorleyNoiseCreateParams detailWorleyParams {};
    detailWorleyParams.numDivisionsRChannel = glm::vec3(7, 15, 22);
    detailWorleyParams.numDivisionsGChannel = glm::vec3(14, 21, 34);
    detailWorleyParams.numDivisionsBChannel = glm::vec3(19, 29, 37);
    detailWorleyParams.numDivisionsAChannel = glm::vec3(33, 45, 49);
    detailWorleyParams.persistenceRChannel = 0.82f;
    detailWorleyParams.persistenceGChannel = 0.73f;
    detailWorleyParams.persistenceBChannel = 0.82f;
    detailWorleyParams.persistenceAChannel = 0.85f;
    detailNoise = std::make_unique<WorleyNoise3D>(glm::vec3(128, 128, 128), detailWorleyParams,
        vDevice, descriptorPool);

    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();

    createComputeSyncObjects();

};

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT debugMessenger)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance,
                              "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
    else
    {
        std::cerr << "Failed to load destroyDebug" << std::endl;
    }
}

Renderer::~Renderer()
{
    /* Manually reset pointers to invoke destructor -> I don't know how else
       to do this since without this a bunch of warnings from the VK validation layers
       about not destroyed objects pop up */
    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        for(auto &buffer : perFrameData[i].buffers)
        {
            buffer.second.reset();
        }
        vkDestroyQueryPool(vDevice->device, perFrameData[i].querryPool, nullptr);
    }
    noise.reset();
    detailNoise.reset();
    imguiImpl.reset();

    for(auto& dsLayout : descriptorLayouts)
    {
        vkDestroyDescriptorSetLayout(vDevice->device, dsLayout.second, nullptr);
    }
    for(auto& image : frameSharedImages)
    {
        image.second.reset();
    }

    vertexBuffer.reset();
    indexBuffer.reset();
    cleanupSwapchain();

    // commonUniformBuffer.reset();
    // vkFreeCommandBuffers(vDevice->device, vDevice->graphicsCommandPool,
    //     static_cast<uint32_t>(postProcessCommandBuffers.size()), postProcessCommandBuffers.data());

    // vkDestroyDescriptorPool(vDevice->device, descriptorPool, nullptr);
    // vkDestroyRenderPass(vDevice->device, renderPass, nullptr);

    // vkDestroyDescriptorSetLayout(vDevice->device, descriptorSetLayout, nullptr);
    // for (auto framebuffer : swapChainFramebuffers)
    // {
    //     vkDestroyFramebuffer(vDevice->device, framebuffer, nullptr);
    // }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(vDevice->device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vDevice->device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vDevice->device, inFlightFences[i], nullptr);
    }

    vDevice.reset();
    if(validationEnabled)
    {
        DestroyDebugUtilsMessengerEXT(instance, nullptr, debugMessenger);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

#pragma region createInstanceUtils
/**
 *  Iterate through all available Instance validations layers and try to match with
 *  layers specified in validationLayers (class private member)
 *  @return true if all required layer names were found in instance properties false othewise
 */
bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }
    return true;
}

/**
 * Get list of all global extensions required by application
 * @return vector of const char* containing names of extensions
 */
std::vector<const char *> getRequiredExtensions(bool enableValidation)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    /* Vulkan needs extension to interface with the window system
       -> GLFW has a functions that returns needed extensions for that */
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    /* NOTE: using constructor vector (first, last)? */
    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);
    /* Vulkan Validation layers extension */
    if (enableValidation)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

/**
 * Check if all provided extensions are supported by instance throw runtime error if
 * unsupported extension is found
 * @param extensions vector of names of extensions we want to check
 */
void checkExtensionSupport(std::vector<const char *> extensions)
{
    /* get number of supported Vulkan extensions*/
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    /* query extension details */
    /* NOTE: vector.data -> returns pointer to the array representing
        vector data, since vector is guaranteed to be continuous */
    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           supportedExtensions.data());
    for (const auto &extension : extensions)
    {
        bool supported = false;
        for (const auto &supportedExtension : supportedExtensions)
        {
            if (strcmp(supportedExtension.extensionName, extension) == 0)
            {
                supported = true;
                break;
            }
        }
        if (supported)
        {
            std::cout << extension << " FOUND " << std::endl;
        }
        else
        {
            throw std::runtime_error("APP::GLFWExtensionSupport::extension support" +
                std::string(" missing for extension ") + std::string(extension));
        }
    }
}

#pragma endregion createInstanceUtils

void Renderer::loadAssets()
{
    frameSharedImages["TerrainEXRHeightMap"] = std::make_unique<VulkanImage>
        (vDevice, "assets/textures/terrain_heightmap.exr", true);

    frameSharedImages["TerrainEXRHeightMap"]->TransitionImageLayout(
        frameSharedImages["TerrainEXRHeightMap"]->format, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    frameSharedImages["TerrainDiffuseImage"] = std::make_unique<VulkanImage>
        (vDevice, "assets/textures/terrain_colormask.png");

    frameSharedImages["TerrainNormalImage"] = std::make_unique<VulkanImage>
        (vDevice, "assets/textures/terrain_normalmap.png");
}

void Renderer::createInstance(bool enableValidation)
{
    if (enableValidation)
    {
        if(!checkValidationLayerSupport())
        {
            throw std::runtime_error("APP:CREATE_INSTANCE::validation layers requested, but not available");
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Sky";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    /* Specify which vulkan extensions and validation layers we want
        to use -> these are GLOBAL for entire program */
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions(enableValidation);
    checkExtensionSupport(extensions);
    /* Specify desired global extensions*/
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    /* Determine global validation layers to enable*/
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidation)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        /* This creates a separate degbug utils messenger specifically for vkCreate and Destroy
            Instance function calls */
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance))
    {
        throw std::runtime_error("APP::CREATE_INSTANCE::failed to create instance");
    }
}

void Renderer::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_SURFACE::Failed to create window surface");
    }
}

void Renderer::createRenderPass()
{
    #pragma region hdrBackbufferPass
    VkAttachmentDescription hdrBackbufferColorImageAttDesc {};
    hdrBackbufferColorImageAttDesc.format = findInMap(perFrameData[0].images,"HDRColor")->format;
    hdrBackbufferColorImageAttDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    hdrBackbufferColorImageAttDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    hdrBackbufferColorImageAttDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    hdrBackbufferColorImageAttDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    hdrBackbufferColorImageAttDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    hdrBackbufferColorImageAttDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    hdrBackbufferColorImageAttDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription hdrBackbufferDepthOneAttachment{};
    hdrBackbufferDepthOneAttachment.format = findInMap(perFrameData[0].images,"HDRDepthOne")->format;
    hdrBackbufferDepthOneAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    hdrBackbufferDepthOneAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    hdrBackbufferDepthOneAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    hdrBackbufferDepthOneAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    hdrBackbufferDepthOneAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    hdrBackbufferDepthOneAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    hdrBackbufferDepthOneAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription hdrBackbufferDepthTwoAttachment{};
    hdrBackbufferDepthTwoAttachment.format = findInMap(perFrameData[0].images,"HDRDepthTwo")->format;
    hdrBackbufferDepthTwoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    hdrBackbufferDepthTwoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    hdrBackbufferDepthTwoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    hdrBackbufferDepthTwoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    hdrBackbufferDepthTwoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    hdrBackbufferDepthTwoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    hdrBackbufferDepthTwoAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::array<VkSubpassDescription, 4> subpassDescriptions{};
    VkAttachmentReference hdrColorReference {};
    hdrColorReference.attachment = 0;
    hdrColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference hdrDepthOneWriteReference{};
    hdrDepthOneWriteReference.attachment = 1;
    hdrDepthOneWriteReference.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkAttachmentReference hdrDepthTwoWriteReference{};
    hdrDepthTwoWriteReference.attachment = 2;
    hdrDepthTwoWriteReference.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkAttachmentReference hdrDepthOneReadReference{};
    hdrDepthOneReadReference.attachment = 1;
    hdrDepthOneReadReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference hdrDepthTwoReadReference{};
    hdrDepthTwoReadReference.attachment = 2;
    hdrDepthTwoReadReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    /* Write into buffer first depth buffer */
    subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[0].colorAttachmentCount = 1;
    subpassDescriptions[0].pColorAttachments = &hdrColorReference;
    subpassDescriptions[0].pDepthStencilAttachment = &hdrDepthOneWriteReference;

    /* Use depth buffer previously written by first subpass for reading */
    subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[1].colorAttachmentCount = 1;
    subpassDescriptions[1].pColorAttachments = &hdrColorReference;
    subpassDescriptions[1].inputAttachmentCount = 1;
    subpassDescriptions[1].pInputAttachments = &hdrDepthOneReadReference;

    /* Subpass 3 needs to write into second depth buffer and read from first one */
    subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[2].colorAttachmentCount = 1;
    subpassDescriptions[2].inputAttachmentCount = 1;
    subpassDescriptions[2].pInputAttachments = &hdrDepthOneReadReference;
    subpassDescriptions[2].pColorAttachments = &hdrColorReference;
    subpassDescriptions[2].pDepthStencilAttachment = &hdrDepthTwoWriteReference;

    /* Subpass 4 needs to read depth buffer written by subpass 2 */
    subpassDescriptions[3].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[3].colorAttachmentCount = 1;
    subpassDescriptions[3].inputAttachmentCount = 1;
    subpassDescriptions[3].pColorAttachments = &hdrColorReference;
    subpassDescriptions[3].pInputAttachments = &hdrDepthTwoReadReference;

    std::array<VkSubpassDependency, 5> subpassDependencies{};

    /* Make sure the transition from initialLayout in Attachment Description
       to layout in subpass attachment reference happens before we write or 
       read anything into it */
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    /* Wait for subpass 0 to finish write into depth map one and color map (WAW Hazard) */
    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstSubpass = 1;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    /* Wait for subpass 1 to finish wirte into color map (WAW Hazard) */
    subpassDependencies[2].srcSubpass = 1;
    subpassDependencies[2].dstSubpass = 2;
    subpassDependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    /* Wait for subpass 2 to finish write into depth map two */
    subpassDependencies[3].srcSubpass = 2;
    subpassDependencies[3].dstSubpass = 3;
    subpassDependencies[3].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[3].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    /* The same as first dependency, but transfer from layout to final layout */
    subpassDependencies[4].srcSubpass = 3;
    subpassDependencies[4].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[4].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[4].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[4].srcAccessMask = 
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[4].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    /*NOTE: Not sure if this should be dependency by region  */
    subpassDependencies[4].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 3> hdrBackbufferAttachments =
        { hdrBackbufferColorImageAttDesc, hdrBackbufferDepthOneAttachment, hdrBackbufferDepthTwoAttachment};
    
    VkRenderPassCreateInfo hdrBackbufferRenderPassInfo{};
    hdrBackbufferRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    hdrBackbufferRenderPassInfo.attachmentCount = static_cast<uint32_t>(hdrBackbufferAttachments.size());
    hdrBackbufferRenderPassInfo.pAttachments = hdrBackbufferAttachments.data();
    hdrBackbufferRenderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    hdrBackbufferRenderPassInfo.pSubpasses = subpassDescriptions.data();
    hdrBackbufferRenderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    hdrBackbufferRenderPassInfo.pDependencies = subpassDependencies.data();

    if (vkCreateRenderPass(vDevice->device, &hdrBackbufferRenderPassInfo, 
        nullptr, &hdrBackbufferPass) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_RENDER_PASS::Failed to create hdrBackbuffer render pass");
    }

    #pragma endregion hdrBackbufferPass

    #pragma region finalRenderPass
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vSwapChain->swapChainImageFormat;
    colorAttachment.samples = vDevice->msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    /* Multisampled images can't be presented to swap chain, that's why 
       we use this resolve color attachment  */
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = vSwapChain->swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    
    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 1;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    std::array<VkAttachmentDescription, 2> attachments =
        {colorAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if (vkCreateRenderPass(vDevice->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_RENDER_PASS::Failed to create render pass");
    }
    #pragma endregion finalRenderPass
}  

void Renderer::createDescriptorSetLayout()
{

    #pragma region terrain
    VkDescriptorSetLayoutBinding heightDSLayoutBinding;
    heightDSLayoutBinding.binding = 0;
    heightDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    heightDSLayoutBinding.descriptorCount = 1;
    heightDSLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    heightDSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding diffuseDSLayoutBinding;
    diffuseDSLayoutBinding.binding = 1;
    diffuseDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffuseDSLayoutBinding.descriptorCount = 1;
    diffuseDSLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseDSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding normalDSLayoutBinding;
    normalDSLayoutBinding.binding = 2;
    normalDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalDSLayoutBinding.descriptorCount = 1;
    normalDSLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalDSLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> terrainTexturesBindings = {
        heightDSLayoutBinding, diffuseDSLayoutBinding, normalDSLayoutBinding
    };
    VkDescriptorSetLayoutCreateInfo terrainLayoutCI{};
    terrainLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    terrainLayoutCI.bindingCount = 3;
    terrainLayoutCI.pBindings = terrainTexturesBindings.data();

    if (vkCreateDescriptorSetLayout(vDevice->device, &terrainLayoutCI,
        nullptr, &descriptorLayouts["TerrainTextures"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create terrainTextures descriptor set layout");
    }
    #pragma endregion terrain

    #pragma region uboCommon
    VkDescriptorSetLayoutBinding uboCommonDSLayoutBinding;
    uboCommonDSLayoutBinding.binding = 0;
    uboCommonDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCommonDSLayoutBinding.descriptorCount = 1;
    uboCommonDSLayoutBinding.stageFlags = 
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT |
        VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo uboCommonDSLayoutCI{};
    uboCommonDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboCommonDSLayoutCI.bindingCount = 1;
    uboCommonDSLayoutCI.pBindings = &uboCommonDSLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &uboCommonDSLayoutCI,
        nullptr, &descriptorLayouts["CommonUBO"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create uboCommon descriptor set layout");
    }
    #pragma endregion uboCommon

    #pragma region uboSkyConstant
    VkDescriptorSetLayoutBinding uboSkyConstantDSLayoutBinding;
    uboSkyConstantDSLayoutBinding.binding = 0;
    uboSkyConstantDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboSkyConstantDSLayoutBinding.descriptorCount = 1;
    uboSkyConstantDSLayoutBinding.stageFlags = 
        VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo uboSkyConstantDSLayoutCI{};
    uboSkyConstantDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboSkyConstantDSLayoutCI.bindingCount = 1;
    uboSkyConstantDSLayoutCI.pBindings = &uboSkyConstantDSLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &uboSkyConstantDSLayoutCI,
        nullptr, &descriptorLayouts["SkyConstantUBO"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create uboSkyConstant descriptor set layout");
    }
    #pragma endregion uboSkyConstant

    #pragma region computeLUTTextures

    VkDescriptorSetLayoutBinding transmittanceLUTDSLayoutBinding;
    transmittanceLUTDSLayoutBinding.binding = 0;
    transmittanceLUTDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    transmittanceLUTDSLayoutBinding.descriptorCount = 1;
    transmittanceLUTDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding multiscatteringLUTDSLayoutBinding;
    multiscatteringLUTDSLayoutBinding.binding = 1;
    multiscatteringLUTDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    multiscatteringLUTDSLayoutBinding.descriptorCount = 1;
    multiscatteringLUTDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding skyViewLUTOutDSLayoutBinding;
    skyViewLUTOutDSLayoutBinding.binding = 2;
    skyViewLUTOutDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    skyViewLUTOutDSLayoutBinding.descriptorCount = 1;
    skyViewLUTOutDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding AEPerpsectiveLUTDSLayoutBinding;
    AEPerpsectiveLUTDSLayoutBinding.binding = 3;
    AEPerpsectiveLUTDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    AEPerpsectiveLUTDSLayoutBinding.descriptorCount = 1;
    AEPerpsectiveLUTDSLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> computeLayoutBindings = {
        transmittanceLUTDSLayoutBinding, multiscatteringLUTDSLayoutBinding,
        skyViewLUTOutDSLayoutBinding, AEPerpsectiveLUTDSLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo computeLayoutCI{};
    computeLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    computeLayoutCI.bindingCount = 4;
    computeLayoutCI.pBindings = computeLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(vDevice->device, &computeLayoutCI,
        nullptr, &descriptorLayouts["ComputeLUTTextures"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create computeLUT descriptor set layout");
    }

    #pragma endregion computeLUTTextures

    #pragma region transmittanceLUT
    VkDescriptorSetLayoutBinding transmittanceLUTDSLayoutBinding_;
    transmittanceLUTDSLayoutBinding_.binding = 0;
    transmittanceLUTDSLayoutBinding_.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    transmittanceLUTDSLayoutBinding_.descriptorCount = 1;
    transmittanceLUTDSLayoutBinding_.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo transmittanceLUTDSLayoutCI{};
    transmittanceLUTDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    transmittanceLUTDSLayoutCI.bindingCount = 1;
    transmittanceLUTDSLayoutCI.pBindings = &transmittanceLUTDSLayoutBinding_;

    if (vkCreateDescriptorSetLayout(vDevice->device, &transmittanceLUTDSLayoutCI,
        nullptr, &descriptorLayouts["TransmittanceLUT"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create transmittanceLUT descriptor set layout");
    }
    #pragma endregion transmittanceLUT

    #pragma region AEPerspectiveLUTDS
    VkDescriptorSetLayoutBinding AEPerpsectiveLUTDSLayoutBinding_{};
    AEPerpsectiveLUTDSLayoutBinding_.binding = 0;
    AEPerpsectiveLUTDSLayoutBinding_.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    AEPerpsectiveLUTDSLayoutBinding_.descriptorCount = 1;
    AEPerpsectiveLUTDSLayoutBinding_.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    AEPerpsectiveLUTDSLayoutBinding_.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo AEPerspectiveLUTDSLayoutCI{};
    AEPerspectiveLUTDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    AEPerspectiveLUTDSLayoutCI.bindingCount = 1;
    AEPerspectiveLUTDSLayoutCI.pBindings = &AEPerpsectiveLUTDSLayoutBinding_;

    if (vkCreateDescriptorSetLayout(vDevice->device, &AEPerspectiveLUTDSLayoutCI,
        nullptr, &descriptorLayouts["AEPerspectiveLUT"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create AEPerspective descriptor set layout");
    }
    #pragma endregion AEPerspectiveLUTDS

    #pragma region worleyNoiseImageDS
    VkDescriptorSetLayoutBinding worleyNoiseImageDSLayoutBinding{};
    worleyNoiseImageDSLayoutBinding.binding = 0;
    worleyNoiseImageDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    worleyNoiseImageDSLayoutBinding.descriptorCount = 1;
    worleyNoiseImageDSLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    worleyNoiseImageDSLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding worleyNoiseImageDetailDSLayoutBinding{};
    worleyNoiseImageDetailDSLayoutBinding.binding = 1;
    worleyNoiseImageDetailDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    worleyNoiseImageDetailDSLayoutBinding.descriptorCount = 1;
    worleyNoiseImageDetailDSLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    worleyNoiseImageDetailDSLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> worleyNoiseBindings = {
        worleyNoiseImageDSLayoutBinding, worleyNoiseImageDetailDSLayoutBinding};

    VkDescriptorSetLayoutCreateInfo worleyNoiseImageDSLayoutCI{};
    worleyNoiseImageDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    worleyNoiseImageDSLayoutCI.bindingCount = 2;
    worleyNoiseImageDSLayoutCI.pBindings = worleyNoiseBindings.data();

    if (vkCreateDescriptorSetLayout(vDevice->device, &worleyNoiseImageDSLayoutCI,
        nullptr, &descriptorLayouts["WorleyNoise"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create worley noise image descriptor set layout");
    }
    #pragma endregion worleyNoiseImageDS

    #pragma region skyViewLUTIn
    VkDescriptorSetLayoutBinding skyViewLutInDsLayoutBinding{};
    skyViewLutInDsLayoutBinding.binding = 0;
    skyViewLutInDsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    skyViewLutInDsLayoutBinding.descriptorCount = 1;
    skyViewLutInDsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    skyViewLutInDsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo skyViewLUTInDSLayoutCI{};
    skyViewLUTInDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    skyViewLUTInDSLayoutCI.bindingCount = 1;
    skyViewLUTInDSLayoutCI.pBindings = &skyViewLutInDsLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &skyViewLUTInDSLayoutCI,
        nullptr, &descriptorLayouts["SkyViewLUT"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create transmittanceLUT descriptor set layout");
    }
    #pragma endregion skyViewLUTIn

    #pragma region depthReadOne
    VkDescriptorSetLayoutBinding depthReadOneDsLayoutBinding{};
    depthReadOneDsLayoutBinding.binding = 0;
    depthReadOneDsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    depthReadOneDsLayoutBinding.descriptorCount = 1;
    depthReadOneDsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    depthReadOneDsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo depthReadOneDSLayoutCI{};
    depthReadOneDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    depthReadOneDSLayoutCI.bindingCount = 1;
    depthReadOneDSLayoutCI.pBindings = &depthReadOneDsLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &depthReadOneDSLayoutCI,
        nullptr, &descriptorLayouts["DepthOne"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create depth read one descriptor set layout");
    }
    #pragma endregion depthReadOne

    #pragma region depthReadTwo
    VkDescriptorSetLayoutBinding depthReadTwoDsLayoutBinding{};
    depthReadTwoDsLayoutBinding.binding = 0;
    depthReadTwoDsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    depthReadTwoDsLayoutBinding.descriptorCount = 1;
    depthReadTwoDsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    depthReadTwoDsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo depthReadTwoDSLayoutCI{};
    depthReadTwoDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    depthReadTwoDSLayoutCI.bindingCount = 1;
    depthReadTwoDSLayoutCI.pBindings = &depthReadTwoDsLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &depthReadTwoDSLayoutCI,
        nullptr, &descriptorLayouts["DepthTwo"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create depth read two descriptor set layout");
    }
    #pragma endregion depthReadTwo

    #pragma region hdrBackbufferIn
    VkDescriptorSetLayoutBinding hdrBackbufferInDsLayoutBinding{};
    hdrBackbufferInDsLayoutBinding.binding = 0;
    hdrBackbufferInDsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hdrBackbufferInDsLayoutBinding.descriptorCount = 1;
    hdrBackbufferInDsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    hdrBackbufferInDsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo hdrBackbufferInDSLayoutCI{};
    hdrBackbufferInDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    hdrBackbufferInDSLayoutCI.bindingCount = 1;
    hdrBackbufferInDSLayoutCI.pBindings = &hdrBackbufferInDsLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &hdrBackbufferInDSLayoutCI,
        nullptr, &descriptorLayouts["HDRBackbuffer"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create hdr backbuffer descriptor set layout");
    }
    #pragma endregion hdrBackbufferIn

    #pragma region histogramStorageBuffer
    VkDescriptorSetLayoutBinding histogramStorageBufLayoutBinding{};
    histogramStorageBufLayoutBinding.binding = 0;
    histogramStorageBufLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    histogramStorageBufLayoutBinding.descriptorCount = 1;
    histogramStorageBufLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    histogramStorageBufLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo histogramStorageBufLayoutCI{};
    histogramStorageBufLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    histogramStorageBufLayoutCI.bindingCount = 1;
    histogramStorageBufLayoutCI.pBindings = &histogramStorageBufLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &histogramStorageBufLayoutCI,
        nullptr, &descriptorLayouts["HistogramSSBO"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create histogramBuffer descriptor set layout");
    }
    #pragma endregion histogramStorageBuffer

    #pragma region avgLumStorageBuffer
    VkDescriptorSetLayoutBinding avgLumStorageBufLayoutBinding{};
    avgLumStorageBufLayoutBinding.binding = 0;
    avgLumStorageBufLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    avgLumStorageBufLayoutBinding.descriptorCount = 1;
    avgLumStorageBufLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    avgLumStorageBufLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo avgLumStorageBufLayoutCI{};
    avgLumStorageBufLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    avgLumStorageBufLayoutCI.bindingCount = 1;
    avgLumStorageBufLayoutCI.pBindings = &avgLumStorageBufLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &avgLumStorageBufLayoutCI,
        nullptr, &descriptorLayouts["AvgLumSSBO"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create averageLuminance descriptor set layout");
    }
    #pragma endregion avgLumStorageBuffer

    #pragma region cloudParamsUbo
    VkDescriptorSetLayoutBinding uboCloudsParamsDSLayoutBinding;
    uboCloudsParamsDSLayoutBinding.binding = 0;
    uboCloudsParamsDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboCloudsParamsDSLayoutBinding.descriptorCount = 1;
    uboCloudsParamsDSLayoutBinding.stageFlags = 
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo uboCloudsDSLayoutCI{};
    uboCloudsDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboCloudsDSLayoutCI.bindingCount = 1;
    uboCloudsDSLayoutCI.pBindings = &uboCloudsParamsDSLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &uboCloudsDSLayoutCI,
        nullptr, &descriptorLayouts["CloudsParamsUBO"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create cloudsParams descriptor set layout");
    }
    #pragma endregion cloudParamsUbo

    #pragma region postProcessParamsUbo
    VkDescriptorSetLayoutBinding uboPostProcessDSLayoutBinding;
    uboPostProcessDSLayoutBinding.binding = 0;
    uboPostProcessDSLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboPostProcessDSLayoutBinding.descriptorCount = 1;
    uboPostProcessDSLayoutBinding.stageFlags = 
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo uboPostProcessDSLayoutCI{};
    uboPostProcessDSLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboPostProcessDSLayoutCI.bindingCount = 1;
    uboPostProcessDSLayoutCI.pBindings = &uboPostProcessDSLayoutBinding;

    if (vkCreateDescriptorSetLayout(vDevice->device, &uboPostProcessDSLayoutCI,
        nullptr, &descriptorLayouts["PostProcessUBO"]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SET_LAYOUT::\
            Failed to create postProcessUbo descriptor set layout");
    }
    #pragma endregion postProcessParamsUbo
}

void Renderer::createPipelines()
{
    #pragma region terrainPassPipeline
    std::vector<VkDescriptorSetLayout> terrainDescriptorSetLayouts = {
            findInMap(descriptorLayouts,"CommonUBO"),
            findInMap(descriptorLayouts,"SkyConstantUBO"),
            findInMap(descriptorLayouts,"TerrainTextures"), 
            findInMap(descriptorLayouts,"TransmittanceLUT")
    };
    auto terrainVertShaderCode = readFile("shaders/build/terrain.vert.spv");
    auto terrainFragShaderCode = readFile("shaders/build/terrain.frag.spv");

    VkShaderModule terrainVertShaderModule = createShaderModule(vDevice, terrainVertShaderCode);
    VkShaderModule terrainFragShaderModule = createShaderModule(vDevice, terrainFragShaderCode);

    std::vector <VkPipelineShaderStageCreateInfo> terrainShaderStages = {
        VulkanPipeline::initVertexShaderStageCI(terrainVertShaderModule),
        VulkanPipeline::initFragmentShaderStageCI(terrainFragShaderModule)
    };

    VkViewport terrainViewport = VulkanPipeline::initViewport(
        0.0f, 0.0f, (float)vSwapChain->swapChainExtent.width,
        (float)vSwapChain->swapChainExtent.height, 0.0f, 1.0f
    );

    VkRect2D terrainScissor{};
    terrainScissor.offset = {0, 0};
    terrainScissor.extent = vSwapChain->swapChainExtent;

    terrainPassPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        2, terrainShaderStages,
        VulkanPipeline::initVertexStageInputStateCI( Vertex::getBindingDescription(),Vertex::getAttributeDescriptions()),
        VulkanPipeline::initInputAssemblyStateCI(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        VulkanPipeline::initViewportStateCI(false, terrainViewport, terrainScissor),
        VulkanPipeline::initRaserizationStateCI(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
            VK_FRONT_FACE_COUNTER_CLOCKWISE),
        VulkanPipeline::initMultisampleStateCI(VK_TRUE, 0.2f, VK_SAMPLE_COUNT_1_BIT),
        VulkanPipeline::initDepthStencilStateCI(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE),
        VulkanPipeline::initColorBlendStateCI(
            VulkanPipeline::initColorBlendAttachment( 
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                VK_FALSE)),              
        VulkanPipeline::initPiplineLayoutCI(4, terrainDescriptorSetLayouts),
        hdrBackbufferPass,
        0);
    #pragma endregion terrainPassPipeline

    #pragma region computePipelines

    #pragma region transmittanceLUTPipeline
    auto transmittanceLUTComputeShaderCode = readFile("shaders/build/transmittanceLUT.spv");
    VkShaderModule transmittanceLUTComputeShaderModule = 
        createShaderModule(vDevice, transmittanceLUTComputeShaderCode);

    std::vector<VkDescriptorSetLayout> transmittanceDSLayouts = {
        findInMap(descriptorLayouts,"CommonUBO"),
        findInMap(descriptorLayouts,"SkyConstantUBO"),
        findInMap(descriptorLayouts,"ComputeLUTTextures")
    };

    transmittanceLUTPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        VulkanPipeline::initPiplineLayoutCI(3, transmittanceDSLayouts),
        VulkanPipeline::initComputeShaderStageCI(transmittanceLUTComputeShaderModule)
    );
    #pragma endregion transmittanceLUTPipeline

    #pragma region multiscatteringLUTPipeline
    auto multiscatteringLUTComputeShaderCode = readFile("shaders/build/multiscatteringLUT.spv");
    VkShaderModule multiscatteringLUTComputeShaderModule = 
        createShaderModule(vDevice, multiscatteringLUTComputeShaderCode);

    std::vector<VkDescriptorSetLayout> multiscatteringDSLayouts = {
        findInMap(descriptorLayouts,"CommonUBO"),
        findInMap(descriptorLayouts,"SkyConstantUBO"),
        findInMap(descriptorLayouts,"ComputeLUTTextures")
    };

    multiscatteringLUTPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        VulkanPipeline::initPiplineLayoutCI(3, multiscatteringDSLayouts),
        VulkanPipeline::initComputeShaderStageCI(multiscatteringLUTComputeShaderModule)
    );
    #pragma endregion multiscatteringLUTPipeline

    #pragma region skyViewLUTPipeline
    auto skyViewLUTComputeShaderCode = readFile("shaders/build/skyviewLUT.spv");
    VkShaderModule skyViewLUTComputeShaderModule = 
        createShaderModule(vDevice, skyViewLUTComputeShaderCode);

    std::vector<VkDescriptorSetLayout> skyViewDSLayouts = {
        findInMap(descriptorLayouts,"CommonUBO"),
        findInMap(descriptorLayouts,"SkyConstantUBO"),
        findInMap(descriptorLayouts,"ComputeLUTTextures")
    };

    skyViewLUTPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        VulkanPipeline::initPiplineLayoutCI(3, skyViewDSLayouts),
        VulkanPipeline::initComputeShaderStageCI(skyViewLUTComputeShaderModule)
    );
    #pragma endregion skyViewLUTPipeline

    #pragma region AEPerspectiveLUTPipeline
    auto AEPerspectiveLUTComputeShaderCode = readFile("shaders/build/aerialPerspectiveLUT.spv");
    VkShaderModule AEPerspectiveLUTComputeShaderModule = 
        createShaderModule(vDevice, AEPerspectiveLUTComputeShaderCode);

    std::vector<VkDescriptorSetLayout> AEPerspectiveDSLayouts = {
        findInMap(descriptorLayouts,"CommonUBO"),
        findInMap(descriptorLayouts,"SkyConstantUBO"),
        findInMap(descriptorLayouts,"ComputeLUTTextures")
    };

    AEPerspectiveLUTPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        VulkanPipeline::initPiplineLayoutCI(3, AEPerspectiveDSLayouts),
        VulkanPipeline::initComputeShaderStageCI(AEPerspectiveLUTComputeShaderModule)
    );
    #pragma endregion AEPerspectiveLUTPipeline

    #pragma region computeHistogramCreatePipeline
    auto histogramComputeShaderCode = readFile("shaders/build/histogram_generate.spv");
    VkShaderModule histogramComputeShaderModule = 
        createShaderModule(vDevice, histogramComputeShaderCode);

    std::vector<VkDescriptorSetLayout> histogramDSLayouts = {
        findInMap(descriptorLayouts,"HDRBackbuffer"), 
        findInMap(descriptorLayouts,"HistogramSSBO"), 
        findInMap(descriptorLayouts,"PostProcessUBO")
    };

    histogramPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        VulkanPipeline::initPiplineLayoutCI(3, histogramDSLayouts),
        VulkanPipeline::initComputeShaderStageCI(histogramComputeShaderModule)
    );
    #pragma endregion computeHistogramCreatePipeline

    #pragma region computeHistogramSumPipeline
    auto sumHistogramComputeShaderCode = readFile("shaders/build/histogram_sum.spv");
    VkShaderModule sumHistogramComputeShaderModule = 
        createShaderModule(vDevice, sumHistogramComputeShaderCode);

    std::vector<VkDescriptorSetLayout> sumHistogramDSLayouts = {
        findInMap(descriptorLayouts,"AvgLumSSBO"), 
        findInMap(descriptorLayouts,"HistogramSSBO"), 
        findInMap(descriptorLayouts,"PostProcessUBO")
    };

    sumHistogramPipeline = std::make_unique<VulkanPipeline>(
        vDevice,
        VulkanPipeline::initPiplineLayoutCI(3, sumHistogramDSLayouts),
        VulkanPipeline::initComputeShaderStageCI(sumHistogramComputeShaderModule)
    );
    #pragma endregion computeHistogramSumPipeline

    #pragma endregion compute_pipelines

    #pragma region drawCloudsPipeline
    auto cloudsVertexShaderCode = readFile("shaders/build/screen_triangle.spv");
    auto cloudsFragmentShaderCode = readFile("shaders/build/draw_clouds.spv");

    VkShaderModule cloudsVertexShaderModule = createShaderModule(vDevice, cloudsVertexShaderCode); 
    VkShaderModule cloudsFragmentShaderModule = createShaderModule(vDevice, cloudsFragmentShaderCode); 

    std::vector<VkPipelineShaderStageCreateInfo> cloudShaderStages = {
        VulkanPipeline::initVertexShaderStageCI(cloudsVertexShaderModule),
        VulkanPipeline::initFragmentShaderStageCI(cloudsFragmentShaderModule)
    };

    VkViewport cloudsPassViewport = VulkanPipeline::initViewport(
        0.0f, 0.0f, (float)vSwapChain->swapChainExtent.width,
        (float)vSwapChain->swapChainExtent.height, 0.0f, 1.0f
    );

    VkRect2D cloudsPassScissor{};
    cloudsPassScissor.offset = {0, 0};
    cloudsPassScissor.extent = vSwapChain->swapChainExtent;

    std::vector<VkDescriptorSetLayout> cloudsDescriptorSetLayouts = { 
        findInMap(descriptorLayouts,"CommonUBO"), 
        findInMap(descriptorLayouts,"SkyConstantUBO"), 
        findInMap(descriptorLayouts,"CloudsParamsUBO"), 
        findInMap(descriptorLayouts,"DepthOne"), 
        findInMap(descriptorLayouts,"WorleyNoise"), 
        findInMap(descriptorLayouts,"TransmittanceLUT"), 
    };

    cloudsPassPipeline = std::make_unique<VulkanPipeline>(  
        vDevice,
        2, cloudShaderStages,                          
        VulkanPipeline::initVertexStageInputStateCI( 
            std::vector<VkVertexInputBindingDescription>(),
            std::vector<VkVertexInputAttributeDescription>()),
        VulkanPipeline::initInputAssemblyStateCI(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        VulkanPipeline::initViewportStateCI(false, cloudsPassViewport, cloudsPassScissor),
        VulkanPipeline::initRaserizationStateCI(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
            VK_FRONT_FACE_CLOCKWISE),
        VulkanPipeline::initMultisampleStateCI(VK_TRUE, 0.2f, VK_SAMPLE_COUNT_1_BIT),
        VulkanPipeline::initDepthStencilStateCI(VK_TRUE, VK_TRUE, VK_COMPARE_OP_ALWAYS, VK_FALSE),
        VulkanPipeline::initColorBlendStateCI(
            VulkanPipeline::initColorBlendAttachmentSrcAlphaDst(
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                VK_TRUE)),
        VulkanPipeline::initPiplineLayoutCI(6, cloudsDescriptorSetLayouts),
        hdrBackbufferPass,
        2);
    #pragma endregion drawCloudsPipeline

    #pragma region drawSkyPipeline
    auto skyVertexShaderCode = readFile("shaders/build/screen_triangle.spv");
    auto skyFragmentShaderCode = readFile("shaders/build/draw_far_sky.spv");

    VkShaderModule skyVertexShaderModule = createShaderModule(vDevice, skyVertexShaderCode); 
    VkShaderModule skyFragmentShaderModule = createShaderModule(vDevice, skyFragmentShaderCode); 

    std::vector<VkPipelineShaderStageCreateInfo> skyShaderStages = {
        VulkanPipeline::initVertexShaderStageCI(skyVertexShaderModule),
        VulkanPipeline::initFragmentShaderStageCI(skyFragmentShaderModule)
    };

    VkViewport skyPassViewport = VulkanPipeline::initViewport(
        0.0f, 0.0f, (float)vSwapChain->swapChainExtent.width,
        (float)vSwapChain->swapChainExtent.height, 0.0f, 1.0f
    );

    VkRect2D skyPassScissor{};
    skyPassScissor.offset = {0, 0};
    skyPassScissor.extent = vSwapChain->swapChainExtent;

    std::vector<VkDescriptorSetLayout> skyDescriptorSetLayouts = {
        findInMap(descriptorLayouts,"CommonUBO"), 
        findInMap(descriptorLayouts,"SkyConstantUBO"), 
        findInMap(descriptorLayouts,"SkyViewLUT"), 
        findInMap(descriptorLayouts,"DepthOne"), 
    };

    farSkyPassPipeline = std::make_unique<VulkanPipeline>(  
        vDevice,
        2, skyShaderStages,                          
        VulkanPipeline::initVertexStageInputStateCI( 
            std::vector<VkVertexInputBindingDescription>(),
            std::vector<VkVertexInputAttributeDescription>()),
        VulkanPipeline::initInputAssemblyStateCI(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        VulkanPipeline::initViewportStateCI(false, skyPassViewport, skyPassScissor),
        VulkanPipeline::initRaserizationStateCI(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
            VK_FRONT_FACE_CLOCKWISE),
        VulkanPipeline::initMultisampleStateCI(VK_TRUE, 0.2f, VK_SAMPLE_COUNT_1_BIT),
        VulkanPipeline::initDepthStencilStateCI(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE),
        VulkanPipeline::initColorBlendStateCI(
            VulkanPipeline::initColorBlendAttachmentAlpha0ignore(
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                VK_TRUE)),
        VulkanPipeline::initPiplineLayoutCI(4, skyDescriptorSetLayouts),
        hdrBackbufferPass,
        1);
    #pragma endregion drawSkyPipeline

    #pragma region drawAEPerspective
    auto aePerspectiveVertexShaderCode = readFile("shaders/build/screen_triangle.spv");
    auto aePerspectiveFragmentShaderCode = readFile("shaders/build/draw_AE_perspective.spv");

    VkShaderModule aePerspectiveVertexShaderModule = createShaderModule(vDevice, aePerspectiveVertexShaderCode); 
    VkShaderModule aePerspectiveFragmentShaderModule = createShaderModule(vDevice, aePerspectiveFragmentShaderCode); 

    std::vector<VkPipelineShaderStageCreateInfo> aePerspectiveShaderStages = {
        VulkanPipeline::initVertexShaderStageCI(aePerspectiveVertexShaderModule),
        VulkanPipeline::initFragmentShaderStageCI(aePerspectiveFragmentShaderModule)
    };

    VkViewport aePerspectivePassViewport = VulkanPipeline::initViewport(
        0.0f, 0.0f, (float)vSwapChain->swapChainExtent.width,
        (float)vSwapChain->swapChainExtent.height, 0.0f, 1.0f
    );

    VkRect2D aePerspectivePassScissor{};
    aePerspectivePassScissor.offset = {0, 0};
    aePerspectivePassScissor.extent = vSwapChain->swapChainExtent;

    std::vector<VkDescriptorSetLayout> aePerspectiveDescriptorSetLayouts = { 
        findInMap(descriptorLayouts,"CommonUBO"), 
        findInMap(descriptorLayouts,"SkyConstantUBO"), 
        findInMap(descriptorLayouts,"DepthTwo"), 
        findInMap(descriptorLayouts,"AEPerspectiveLUT"), 
    };

    aePerspectivePassPipeline = std::make_unique<VulkanPipeline>(  
        vDevice,
        2, aePerspectiveShaderStages,                          
        VulkanPipeline::initVertexStageInputStateCI( 
            std::vector<VkVertexInputBindingDescription>(),
            std::vector<VkVertexInputAttributeDescription>()),
        VulkanPipeline::initInputAssemblyStateCI(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        VulkanPipeline::initViewportStateCI(false, aePerspectivePassViewport, aePerspectivePassScissor),
        VulkanPipeline::initRaserizationStateCI(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
            VK_FRONT_FACE_CLOCKWISE),
        VulkanPipeline::initMultisampleStateCI(VK_TRUE, 0.2f, VK_SAMPLE_COUNT_1_BIT),
        VulkanPipeline::initDepthStencilStateCI(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE),
        VulkanPipeline::initColorBlendStateCI(
            VulkanPipeline::initColorBlendAttachment(
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                VK_TRUE)),
        VulkanPipeline::initPiplineLayoutCI(4, aePerspectiveDescriptorSetLayouts),
        hdrBackbufferPass,
        3);
    #pragma endregion drawAEPerspective

    #pragma region final_pass_pipeline
    auto vertexShaderCode = readFile("shaders/build/screen_triangle.spv");
    auto fragmentShaderCode = readFile("shaders/build/final_composition.spv");

    VkShaderModule vertexShaderModule = createShaderModule(vDevice, vertexShaderCode); 
    VkShaderModule fragmentShaderModule = createShaderModule(vDevice, fragmentShaderCode); 

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
        VulkanPipeline::initVertexShaderStageCI(vertexShaderModule),
        VulkanPipeline::initFragmentShaderStageCI(fragmentShaderModule)
    };

    VkViewport finalPassViewport = VulkanPipeline::initViewport(
        0.0f, 0.0f, (float)vSwapChain->swapChainExtent.width,
        (float)vSwapChain->swapChainExtent.height, 0.0f, 1.0f
    );

    VkRect2D finalPassScissor{};
    finalPassScissor.offset = {0, 0};
    finalPassScissor.extent = vSwapChain->swapChainExtent;

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
        findInMap(descriptorLayouts,"HDRBackbuffer"), 
        findInMap(descriptorLayouts,"AvgLumSSBO"), 
        findInMap(descriptorLayouts,"PostProcessUBO"), 
    };

    finalPassPipeline = std::make_unique<VulkanPipeline>(  
        vDevice,
        2, shaderStages,                          
        VulkanPipeline::initVertexStageInputStateCI( 
            std::vector<VkVertexInputBindingDescription>(),
            std::vector<VkVertexInputAttributeDescription>()),
        VulkanPipeline::initInputAssemblyStateCI(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        VulkanPipeline::initViewportStateCI(false, finalPassViewport, finalPassScissor),
        VulkanPipeline::initRaserizationStateCI(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
            VK_FRONT_FACE_CLOCKWISE),
        VulkanPipeline::initMultisampleStateCI(VK_TRUE, 0.2f, vDevice->msaaSamples),
        VulkanPipeline::initDepthStencilStateCI(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE),
        VulkanPipeline::initColorBlendStateCI(
            VulkanPipeline::initColorBlendAttachment(
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                VK_FALSE)),
        VulkanPipeline::initPiplineLayoutCI(3, descriptorSetLayouts),
        renderPass,
        0);

    #pragma endregion final_pass_pipeline
}

void Renderer::createAttachments()
{
    VkFormat depthFormat = vDevice->findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        perFrameData[i].images["HDRColor"] = std::make_unique<VulkanImage>(
            vDevice, vSwapChain->swapChainExtent.width, vSwapChain->swapChainExtent.height,
            1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        perFrameData[i].images["HDRDepthOne"] = std::make_unique<VulkanImage>(
            vDevice ,vSwapChain->swapChainExtent.width, vSwapChain->swapChainExtent.height,
            1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    
        perFrameData[i].images["HDRDepthTwo"] = std::make_unique<VulkanImage>(
            vDevice ,vSwapChain->swapChainExtent.width, vSwapChain->swapChainExtent.height,
            1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    frameSharedImages["ColorImage"] = std::make_unique<VulkanImage>(vDevice ,vSwapChain->swapChainExtent.width, 
        vSwapChain->swapChainExtent.height, 1,
        vDevice->msaaSamples, vSwapChain->swapChainImageFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Renderer::createFramebuffers()
{
    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        #pragma region offscreenFramebuffer
        std::array<VkImageView, 3> offscreenAttachments =
        {
            findInMap(perFrameData[i].images,"HDRColor")->imageView,
            findInMap(perFrameData[i].images,"HDRDepthOne")->imageView,
            findInMap(perFrameData[i].images,"HDRDepthTwo")->imageView
        };

        VkFramebufferCreateInfo offscreenFramebufferCreateInfo {};
        offscreenFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        offscreenFramebufferCreateInfo.renderPass = hdrBackbufferPass;
        offscreenFramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(offscreenAttachments.size());
        offscreenFramebufferCreateInfo.pAttachments = offscreenAttachments.data();
        offscreenFramebufferCreateInfo.width = vSwapChain->swapChainExtent.width;
        offscreenFramebufferCreateInfo.height = vSwapChain->swapChainExtent.height;
        offscreenFramebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(vDevice->device, &offscreenFramebufferCreateInfo,
            nullptr, &perFrameData[i].framebuffers["Offscreen"]) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_FRAMEBUFFERS::\
                Failed to create offscreen framebuffer");
        }
        #pragma endregion offscreenFramebuffer

        #pragma region imguiPassFramebuffer
        std::array<VkImageView, 1> attachments =
        {   
            vSwapChain->swapChainImageViews[i],
        };
    
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        /* Specify which renderPass the framebuffer needs to be compatible with */
        framebufferInfo.renderPass = imguiImpl->imguiRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = vSwapChain->swapChainExtent.width;
        framebufferInfo.height = vSwapChain->swapChainExtent.height;
        framebufferInfo.layers = 1;
    
        if (vkCreateFramebuffer(vDevice->device, &framebufferInfo,
            nullptr, &perFrameData[i].framebuffers["ImGui"]) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_FRAMEBUFFERS::Failed to create framebuffer for imgui pass");
        }
        #pragma endregion imguiPassFramebuffer

        #pragma region finalPassFramebuffer
        std::array<VkImageView, 2> finalPassAttachments =
        {   
            findInMap(frameSharedImages,"ColorImage")->imageView,
            vSwapChain->swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferFinalPassInfo{};
        framebufferFinalPassInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        /* Specify which renderPass the framebuffer needs to be compatible with */
        framebufferFinalPassInfo.renderPass = renderPass;
        framebufferFinalPassInfo.attachmentCount = static_cast<uint32_t>(finalPassAttachments.size());
        framebufferFinalPassInfo.pAttachments = finalPassAttachments.data();
        framebufferFinalPassInfo.width = vSwapChain->swapChainExtent.width;
        framebufferFinalPassInfo.height = vSwapChain->swapChainExtent.height;
        framebufferFinalPassInfo.layers = 1;

        if (vkCreateFramebuffer(vDevice->device, &framebufferFinalPassInfo,
            nullptr, &perFrameData[i].framebuffers["FinalPass"]) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_FRAMEBUFFERS::Failed to create framebuffer");
        }
        #pragma endregion finalPassFramebuffer
    }
}

void Renderer::createPrimitivesBuffers()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const unsigned int terrainRes = 3000;

    #pragma region primitivesGeneration
    /* Generate uniform plane filled with vertices */
    for (unsigned int i = 0; i < terrainRes; i++) {
        for (unsigned int j = 0; j < terrainRes; j++) {
            Vertex vertex;
            glm::vec3 position = glm::vec3((float(i) / (terrainRes - 1)),
                                           (float(j) / (terrainRes - 1)),
                                           (0));
            /* Texture coords are the same as position, since the generated plane is always unit len*/
            glm::vec2 textureCoords = glm::vec2(position.x, position.y);
            vertex.pos = position;
            vertex.texCoord = textureCoords;
            vertices.push_back(vertex);
        }
    }

    /* Generate indices for above generated uniform plane */
    for (unsigned int i = 0; i < terrainRes - 1; i++) {
        for (unsigned int j = 0; j < terrainRes - 1; j++) {
            int i0 = j + i * terrainRes;
            int i1 = i0 + 1;
            int i2 = i0 + terrainRes;
            int i3 = i2 + 1;
            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    #pragma endregion primitivesGeneration

    #pragma region vertexBuffer
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VulkanBuffer stagingBuffer = VulkanBuffer(vDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    /* Filling staging buffer */
    void *data;
    /* Allows access to a region of the specified memory resource defined by an offset and size*/
    /* TODO: Make this function of buffer */
    vkMapMemory(vDevice->device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(vDevice->device, stagingBuffer.bufferMemory);

    vertexBuffer = std::make_unique<VulkanBuffer>(vDevice,bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    /* copy staging buffer into vertex buffer on GPU*/
    vertexBuffer->CopyIntoBuffer(stagingBuffer, bufferSize);
    #pragma endregion vertexBuffer

    #pragma region indexBuffer
    std::cout << "index buffer size is: " << indices.size() << std::endl;
    bufferSize = sizeof(indices[0]) * indices.size();

    VulkanBuffer stagingIndexBuffer = VulkanBuffer(vDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    /* Allows access to a region of the specified memory resource defined by an offset and size*/
    vkMapMemory(vDevice->device, stagingIndexBuffer.bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(vDevice->device, stagingIndexBuffer.bufferMemory);
    
    indexBuffer = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    /* copy staging buffer into vertex buffer on GPU*/
    indexBuffer->CopyIntoBuffer(stagingIndexBuffer, bufferSize);
    #pragma endregion indexBuffer
}

void Renderer::createUniformBuffers()
{
    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        perFrameData[i].buffers["CommonUBO"] = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        bufferSize = sizeof(AtmosphereParametersBuffer);
        /* TODO -> Since this buffer will only be very sparcely change, it probably should
                   be only DEVICE LOCAL and updated via staging buffer */
        perFrameData[i].buffers["SkyConstantUBO"] = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        bufferSize = sizeof(CloudsParametersBuffer);
        perFrameData[i].buffers["CloudsParamsUBO"] = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        bufferSize = sizeof(PostProcessParamsBuffer);
        perFrameData[i].buffers["PostProcessUBO"] = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        bufferSize = sizeof(uint32_t) * 256;
        perFrameData[i].buffers["HistogramSSBO"] = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VulkanBuffer stagingBuffer_ = VulkanBuffer(vDevice, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        vkMapMemory(vDevice->device, stagingBuffer_.bufferMemory, 0, bufferSize, 0, &data);
        memset(data, 0, bufferSize);
        vkUnmapMemory(vDevice->device, stagingBuffer_.bufferMemory);
        
        findInMap(perFrameData[i].buffers, "HistogramSSBO")->CopyIntoBuffer(stagingBuffer_, bufferSize);


        bufferSize = sizeof(float);
        perFrameData[i].buffers["AvgLumSSBO"] = std::make_unique<VulkanBuffer>(vDevice, bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VulkanBuffer stagingBuffer = VulkanBuffer(vDevice, bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkMapMemory(vDevice->device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
        memset(data, 0, bufferSize);
        vkUnmapMemory(vDevice->device, stagingBuffer.bufferMemory);
        
        findInMap(perFrameData[i].buffers, "AvgLumSSBO")->CopyIntoBuffer(stagingBuffer, bufferSize);
    }
}

void Renderer::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 5> poolSizes{};
    // uboCommon and SkyConstant buffers
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 50;
    // Graphics pipeline sampler for displaying compute output image -> SkyViewLUTIn
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 50;
    // Compute pipeline storage image for reads and writes -> 
    // Transmittance, Multiscattering and SkyView LUTs
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[2].descriptorCount = 50;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[3].descriptorCount = 50;
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[4].descriptorCount = 50;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    /* maximum number of descriptor sets that may be allocated */
    poolInfo.maxSets = 70;

    if (vkCreateDescriptorPool(vDevice->device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_DESCRIPTOR_POOL::Failed to create descriptor pool");
    }
}

void Renderer::createDescriptorSets()
{
    #pragma region frameIndependentResources
    std::vector<VkDescriptorSetLayout> layoutsToBeAllocated = {
        findInMap(descriptorLayouts, "TerrainTextures"),
        findInMap(descriptorLayouts, "WorleyNoise")
    };

    std::array<VkDescriptorSet,2> targetDescriptorSets;

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = 2;
    allocateInfo.pSetLayouts = layoutsToBeAllocated.data();

    if (vkAllocateDescriptorSets(vDevice->device, &allocateInfo, targetDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SETS::Failed to allocate frame independent sets");
    }
    frameSharedDS["TerrainTextures"]          = targetDescriptorSets[0];
    frameSharedDS["WorleyNoise"]              = targetDescriptorSets[1];

    VkDescriptorImageInfo heightMapImageInfo{};
    heightMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // heightMapImageInfo.imageView = terrainHeightImage->imageView;
    heightMapImageInfo.imageView = findInMap(frameSharedImages,"TerrainEXRHeightMap")->imageView;
    heightMapImageInfo.sampler = terrainTexturesSampler;

    VkDescriptorImageInfo diffuseMapImageInfo{};
    diffuseMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    diffuseMapImageInfo.imageView = findInMap(frameSharedImages,"TerrainDiffuseImage")->imageView;
    diffuseMapImageInfo.sampler = terrainTexturesSampler;

    VkDescriptorImageInfo normalMapImageInfo{};
    normalMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normalMapImageInfo.imageView = findInMap(frameSharedImages,"TerrainNormalImage")->imageView;
    normalMapImageInfo.sampler = terrainTexturesSampler;

    VkDescriptorImageInfo worleyNoiseDetailImageInfo{};
    worleyNoiseDetailImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    worleyNoiseDetailImageInfo.imageView = detailNoise->noiseImage->imageView;
    worleyNoiseDetailImageInfo.sampler = cloudsSampler;

    VkDescriptorImageInfo worleyNoiseImageInfo{};
    worleyNoiseImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    worleyNoiseImageInfo.imageView = noise->noiseImage->imageView;
    worleyNoiseImageInfo.sampler = cloudsSampler;

    std::array<VkWriteDescriptorSet, 5> updateDescriptorWrites{};
    updateDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    updateDescriptorWrites[0].dstSet = findInMap(frameSharedDS, "TerrainTextures");
    updateDescriptorWrites[0].dstBinding = 0;
    updateDescriptorWrites[0].dstArrayElement = 0;
    updateDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    updateDescriptorWrites[0].descriptorCount = 1;
    updateDescriptorWrites[0].pImageInfo = &heightMapImageInfo;

    updateDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    updateDescriptorWrites[1].dstSet = findInMap(frameSharedDS, "TerrainTextures");
    updateDescriptorWrites[1].dstBinding = 1;
    updateDescriptorWrites[1].dstArrayElement = 0;
    updateDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    updateDescriptorWrites[1].descriptorCount = 1;
    updateDescriptorWrites[1].pImageInfo = &diffuseMapImageInfo;

    updateDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    updateDescriptorWrites[2].dstSet = findInMap(frameSharedDS, "TerrainTextures");
    updateDescriptorWrites[2].dstBinding = 2;
    updateDescriptorWrites[2].dstArrayElement = 0;
    updateDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    updateDescriptorWrites[2].descriptorCount = 1;
    updateDescriptorWrites[2].pImageInfo = &normalMapImageInfo;

    updateDescriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    updateDescriptorWrites[3].dstSet = findInMap(frameSharedDS, "WorleyNoise");
    updateDescriptorWrites[3].dstBinding = 0;
    updateDescriptorWrites[3].dstArrayElement = 0;
    updateDescriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    updateDescriptorWrites[3].descriptorCount = 1;
    updateDescriptorWrites[3].pImageInfo = &worleyNoiseImageInfo;

    updateDescriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    updateDescriptorWrites[4].dstSet = findInMap(frameSharedDS, "WorleyNoise");
    updateDescriptorWrites[4].dstBinding = 1;
    updateDescriptorWrites[4].dstArrayElement = 0;
    updateDescriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    updateDescriptorWrites[4].descriptorCount = 1;
    updateDescriptorWrites[4].pImageInfo = &worleyNoiseDetailImageInfo;

    vkUpdateDescriptorSets(vDevice->device, static_cast<uint32_t>(updateDescriptorWrites.size()),
                            updateDescriptorWrites.data(), 0, nullptr);
    #pragma endregion frameIndependentResources

    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        std::vector<VkDescriptorSetLayout> layoutsToBeAllocated = {
            findInMap(descriptorLayouts, "CommonUBO"),
            findInMap(descriptorLayouts, "SkyConstantUBO"),
            findInMap(descriptorLayouts, "PostProcessUBO"),
            findInMap(descriptorLayouts, "CloudsParamsUBO"),
            findInMap(descriptorLayouts, "HistogramSSBO"),
            findInMap(descriptorLayouts, "AvgLumSSBO"),
            findInMap(descriptorLayouts, "TransmittanceLUT"),
            findInMap(descriptorLayouts, "SkyViewLUT"),
            findInMap(descriptorLayouts, "AEPerspectiveLUT"),
            findInMap(descriptorLayouts, "ComputeLUTTextures"),
            findInMap(descriptorLayouts, "HDRBackbuffer"),
            findInMap(descriptorLayouts, "DepthOne"),
            findInMap(descriptorLayouts, "DepthTwo")
        };

        std::array<VkDescriptorSet,13> targetDescriptorSets;

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = descriptorPool;
        allocateInfo.descriptorSetCount = 13;
        allocateInfo.pSetLayouts = layoutsToBeAllocated.data();

        if (vkAllocateDescriptorSets(vDevice->device, &allocateInfo, targetDescriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_DESCRIPTOR_SETS::Failed to allocate descriptor sets");
        }

        perFrameData[i].descriptorSets["CommonUBO"]          = targetDescriptorSets[0];
        perFrameData[i].descriptorSets["SkyConstantUBO"]     = targetDescriptorSets[1];
        perFrameData[i].descriptorSets["PostProcessUBO"]     = targetDescriptorSets[2];
        perFrameData[i].descriptorSets["CloudsParamsUBO"]    = targetDescriptorSets[3];
        perFrameData[i].descriptorSets["HistogramSSBO"]      = targetDescriptorSets[4];
        perFrameData[i].descriptorSets["AvgLumSSBO"]         = targetDescriptorSets[5];
        perFrameData[i].descriptorSets["TransmittanceLUT"]   = targetDescriptorSets[6];
        perFrameData[i].descriptorSets["SkyViewLUT"]         = targetDescriptorSets[7];
        perFrameData[i].descriptorSets["AEPerspectiveLUT"]   = targetDescriptorSets[8];
        perFrameData[i].descriptorSets["ComputeLUTTextures"] = targetDescriptorSets[9];
        perFrameData[i].descriptorSets["HDRBackbuffer"]      = targetDescriptorSets[10];
        perFrameData[i].descriptorSets["DepthOne"]           = targetDescriptorSets[11];
        perFrameData[i].descriptorSets["DepthTwo"]           = targetDescriptorSets[12];

        VkDescriptorBufferInfo uboCommonBufferInfo{};
        uboCommonBufferInfo.buffer = findInMap(perFrameData[i].buffers,"CommonUBO")->buffer;
        uboCommonBufferInfo.offset = 0;
        uboCommonBufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo uboSkyAtmosphereBufferInfo{};
        uboSkyAtmosphereBufferInfo.buffer = findInMap(perFrameData[i].buffers,"SkyConstantUBO")->buffer;
        uboSkyAtmosphereBufferInfo.offset = 0;
        uboSkyAtmosphereBufferInfo.range = sizeof(AtmosphereParametersBuffer);

        VkDescriptorBufferInfo postProcessUboInfo{};
        postProcessUboInfo.buffer = findInMap(perFrameData[i].buffers,"PostProcessUBO")->buffer;
        postProcessUboInfo.offset = 0;
        postProcessUboInfo.range = sizeof(PostProcessParamsBuffer);
        
        VkDescriptorBufferInfo cloudsParamsUboInfo{};
        cloudsParamsUboInfo.buffer = findInMap(perFrameData[i].buffers,"CloudsParamsUBO")->buffer;
        cloudsParamsUboInfo.offset = 0;
        cloudsParamsUboInfo.range = sizeof(CloudsParametersBuffer);

        VkDescriptorBufferInfo histogramSSBOInfo{};
        histogramSSBOInfo.buffer = findInMap(perFrameData[i].buffers,"HistogramSSBO")->buffer;
        histogramSSBOInfo.offset = 0;
        histogramSSBOInfo.range = sizeof(uint32_t) * 256;

        VkDescriptorBufferInfo avgLumSSBOInfo{};
        avgLumSSBOInfo.buffer = findInMap(perFrameData[i].buffers,"AvgLumSSBO")->buffer;
        avgLumSSBOInfo.offset = 0;
        avgLumSSBOInfo.range = sizeof(float);

        VkDescriptorImageInfo transmittanceLUTImageInfo{};
        transmittanceLUTImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        transmittanceLUTImageInfo.imageView = 
            findInMap(perFrameData[i].images,"TransmittanceLUT")->imageView;

        VkDescriptorImageInfo multiscatteringLUTImageInfo{};
        multiscatteringLUTImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        multiscatteringLUTImageInfo.imageView = 
            findInMap(perFrameData[i].images,"MultiscatteringLUT")->imageView;

        VkDescriptorImageInfo skyViewLUTOutImageInfo{};
        skyViewLUTOutImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        skyViewLUTOutImageInfo.imageView = 
            findInMap(perFrameData[i].images,"SkyViewLUT")->imageView;

        VkDescriptorImageInfo skyViewLUTInImageInfo{};
        skyViewLUTInImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        skyViewLUTInImageInfo.imageView = 
            findInMap(perFrameData[i].images,"SkyViewLUT")->imageView;
        skyViewLUTInImageInfo.sampler = skyViewLUTSampler;

        VkDescriptorImageInfo AEPerspectiveLUTImageInfo{};
        AEPerspectiveLUTImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        AEPerspectiveLUTImageInfo.imageView = 
            findInMap(perFrameData[i].images,"AEPerspectiveLUT")->imageView;
        AEPerspectiveLUTImageInfo.sampler = AEPerspectiveSampler;

        VkDescriptorImageInfo hdrBackbufferInImageInfo{};
        hdrBackbufferInImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        hdrBackbufferInImageInfo.imageView = 
            findInMap(perFrameData[i].images,"HDRColor")->imageView;
        hdrBackbufferInImageInfo.sampler = skyViewLUTSampler;

        VkDescriptorImageInfo depthOneImageInfo{};
        depthOneImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthOneImageInfo.imageView = findInMap(perFrameData[i].images,"HDRDepthOne")->imageView;
        depthOneImageInfo.sampler = VK_NULL_HANDLE;

        VkDescriptorImageInfo depthTwoImageInfo{};
        depthTwoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthTwoImageInfo.imageView = findInMap(perFrameData[i].images,"HDRDepthTwo")->imageView;
        depthTwoImageInfo.sampler = VK_NULL_HANDLE;

        std::array<VkWriteDescriptorSet, 16> updateDescriptorWrites{};
        updateDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[0].dstSet = findInMap(perFrameData[i].descriptorSets, "CommonUBO");
        updateDescriptorWrites[0].dstBinding = 0;
        updateDescriptorWrites[0].dstArrayElement = 0;
        updateDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        updateDescriptorWrites[0].descriptorCount = 1;
        updateDescriptorWrites[0].pBufferInfo = &uboCommonBufferInfo;

        updateDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[1].dstSet = findInMap(perFrameData[i].descriptorSets, "SkyConstantUBO");
        updateDescriptorWrites[1].dstBinding = 0;
        updateDescriptorWrites[1].dstArrayElement = 0;
        updateDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        updateDescriptorWrites[1].descriptorCount = 1;
        updateDescriptorWrites[1].pBufferInfo = &uboSkyAtmosphereBufferInfo;

        updateDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[2].dstSet = findInMap(perFrameData[i].descriptorSets, "PostProcessUBO");
        updateDescriptorWrites[2].dstBinding = 0;
        updateDescriptorWrites[2].dstArrayElement = 0;
        updateDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        updateDescriptorWrites[2].descriptorCount = 1;
        updateDescriptorWrites[2].pBufferInfo = &postProcessUboInfo;

        updateDescriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[3].dstSet = findInMap(perFrameData[i].descriptorSets, "CloudsParamsUBO");
        updateDescriptorWrites[3].dstBinding = 0;
        updateDescriptorWrites[3].dstArrayElement = 0;
        updateDescriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        updateDescriptorWrites[3].descriptorCount = 1;
        updateDescriptorWrites[3].pBufferInfo = &cloudsParamsUboInfo;

        updateDescriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[4].dstSet = findInMap(perFrameData[i].descriptorSets, "HistogramSSBO");
        updateDescriptorWrites[4].dstBinding = 0;
        updateDescriptorWrites[4].dstArrayElement = 0;
        updateDescriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        updateDescriptorWrites[4].descriptorCount = 1;
        updateDescriptorWrites[4].pBufferInfo = &histogramSSBOInfo;

        updateDescriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[5].dstSet = findInMap(perFrameData[i].descriptorSets, "AvgLumSSBO");
        updateDescriptorWrites[5].dstBinding = 0;
        updateDescriptorWrites[5].dstArrayElement = 0;
        updateDescriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        updateDescriptorWrites[5].descriptorCount = 1;
        updateDescriptorWrites[5].pBufferInfo = &avgLumSSBOInfo;

        updateDescriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[6].dstSet = findInMap(perFrameData[i].descriptorSets, "TransmittanceLUT");
        updateDescriptorWrites[6].dstBinding = 0;
        updateDescriptorWrites[6].dstArrayElement = 0;
        updateDescriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        updateDescriptorWrites[6].descriptorCount = 1;
        updateDescriptorWrites[6].pImageInfo = &transmittanceLUTImageInfo;

        updateDescriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[7].dstSet = findInMap(perFrameData[i].descriptorSets, "SkyViewLUT");
        updateDescriptorWrites[7].dstBinding = 0;
        updateDescriptorWrites[7].dstArrayElement = 0;
        updateDescriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        updateDescriptorWrites[7].descriptorCount = 1;
        updateDescriptorWrites[7].pImageInfo = &skyViewLUTInImageInfo;

        updateDescriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[8].dstSet = findInMap(perFrameData[i].descriptorSets, "AEPerspectiveLUT");
        updateDescriptorWrites[8].dstBinding = 0;
        updateDescriptorWrites[8].dstArrayElement = 0;
        updateDescriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        updateDescriptorWrites[8].descriptorCount = 1;
        updateDescriptorWrites[8].pImageInfo = &AEPerspectiveLUTImageInfo;

        updateDescriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[9].dstSet = findInMap(perFrameData[i].descriptorSets, "ComputeLUTTextures");
        updateDescriptorWrites[9].dstBinding = 0;
        updateDescriptorWrites[9].dstArrayElement = 0;
        updateDescriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        updateDescriptorWrites[9].descriptorCount = 1;
        updateDescriptorWrites[9].pImageInfo = &transmittanceLUTImageInfo;

        updateDescriptorWrites[10].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[10].dstSet = findInMap(perFrameData[i].descriptorSets, "ComputeLUTTextures");
        updateDescriptorWrites[10].dstBinding = 1;
        updateDescriptorWrites[10].dstArrayElement = 0;
        updateDescriptorWrites[10].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        updateDescriptorWrites[10].descriptorCount = 1;
        updateDescriptorWrites[10].pImageInfo = &multiscatteringLUTImageInfo;

        updateDescriptorWrites[11].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[11].dstSet = findInMap(perFrameData[i].descriptorSets, "ComputeLUTTextures");
        updateDescriptorWrites[11].dstBinding = 2;
        updateDescriptorWrites[11].dstArrayElement = 0;
        updateDescriptorWrites[11].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        updateDescriptorWrites[11].descriptorCount = 1;
        updateDescriptorWrites[11].pImageInfo = &skyViewLUTOutImageInfo;

        updateDescriptorWrites[12].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[12].dstSet = findInMap(perFrameData[i].descriptorSets, "ComputeLUTTextures");
        updateDescriptorWrites[12].dstBinding = 3;
        updateDescriptorWrites[12].dstArrayElement = 0;
        updateDescriptorWrites[12].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        updateDescriptorWrites[12].descriptorCount = 1;
        updateDescriptorWrites[12].pImageInfo = &AEPerspectiveLUTImageInfo;

        updateDescriptorWrites[13].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[13].dstSet = findInMap(perFrameData[i].descriptorSets, "HDRBackbuffer");
        updateDescriptorWrites[13].dstBinding = 0;
        updateDescriptorWrites[13].dstArrayElement = 0;
        updateDescriptorWrites[13].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        updateDescriptorWrites[13].descriptorCount = 1;
        updateDescriptorWrites[13].pImageInfo = &hdrBackbufferInImageInfo;

        updateDescriptorWrites[14].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[14].dstSet = findInMap(perFrameData[i].descriptorSets, "DepthOne");
        updateDescriptorWrites[14].dstBinding = 0;
        updateDescriptorWrites[14].dstArrayElement = 0;
        updateDescriptorWrites[14].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        updateDescriptorWrites[14].descriptorCount = 1;
        updateDescriptorWrites[14].pImageInfo = &depthOneImageInfo;

        updateDescriptorWrites[15].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        updateDescriptorWrites[15].dstSet = findInMap(perFrameData[i].descriptorSets, "DepthTwo");
        updateDescriptorWrites[15].dstBinding = 0;
        updateDescriptorWrites[15].dstArrayElement = 0;
        updateDescriptorWrites[15].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        updateDescriptorWrites[15].descriptorCount = 1;
        updateDescriptorWrites[15].pImageInfo = &depthTwoImageInfo;
        vkUpdateDescriptorSets(vDevice->device, static_cast<uint32_t>(updateDescriptorWrites.size()),
                               updateDescriptorWrites.data(), 0, nullptr);
    }
}

/* TODO: This should be done more consistently with device design 
         Think about better solution */ 
void Renderer::createCommandBuffers() {

    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        perFrameData[i].commandBuffers["ComputeLUTs"] = vDevice->createGraphicsCommandBuffer();
        perFrameData[i].commandBuffers["RenderSky"] = vDevice->createGraphicsCommandBuffer();
        perFrameData[i].commandBuffers["PostProcess"] = vDevice->createGraphicsCommandBuffer();

        #pragma region LUTs
        VkCommandBuffer computeLUTsCommandBuffer = 
            findInMap(perFrameData[i].commandBuffers,"ComputeLUTs");

        VkCommandBufferBeginInfo LUTsCommandBufferBI {};
        LUTsCommandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if(vkBeginCommandBuffer(computeLUTsCommandBuffer, &LUTsCommandBufferBI) 
            != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::BUILD_COMPUTE_COMMAND_BUFFER::\
                Failed begin multiscattering compute command buffer");
        }

        #pragma region transmittanceLUT
        vkCmdBindPipeline(computeLUTsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            transmittanceLUTPipeline->pipeline);
        std::array<VkDescriptorSet, 3> LUTDescriptorSets = {
            findInMap(perFrameData[i].descriptorSets,"CommonUBO"),
            findInMap(perFrameData[i].descriptorSets,"SkyConstantUBO"),
            findInMap(perFrameData[i].descriptorSets,"ComputeLUTTextures")
        };
        vkCmdBindDescriptorSets(computeLUTsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            transmittanceLUTPipeline->layout, 0, 3, LUTDescriptorSets.data(), 0, nullptr);

        vkCmdResetQueryPool(computeLUTsCommandBuffer, perFrameData[i].querryPool, 0, 30);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 0);
        vkCmdDispatch(computeLUTsCommandBuffer, 256/8, 64/4, 1);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 1);
        #pragma endregion transmittanceLUT

        VkMemoryBarrier prevComputeWorkFinished = {};
        prevComputeWorkFinished.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        prevComputeWorkFinished.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        prevComputeWorkFinished.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            computeLUTsCommandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1,
            &prevComputeWorkFinished, 
            0, nullptr,
            0, nullptr
        );

        #pragma region multiscatteringLUT
        vkCmdBindPipeline(computeLUTsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
            multiscatteringLUTPipeline->pipeline);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 2);
        vkCmdDispatch(computeLUTsCommandBuffer, 32, 32, 1);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 3);
        #pragma endregion multiscatteringLUT


        vkCmdPipelineBarrier(
            computeLUTsCommandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1,
            &prevComputeWorkFinished, 
            0, nullptr,
            0, nullptr
        );

        #pragma region skyViewLUT
        vkCmdBindPipeline(computeLUTsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
            skyViewLUTPipeline->pipeline);

        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 4);
        vkCmdDispatch(computeLUTsCommandBuffer, 192/16, 128/16, 1);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 5);
        #pragma endregion skyViewLUT

        #pragma region AEPerspectiveLUT
        vkCmdBindPipeline(computeLUTsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            AEPerspectiveLUTPipeline->pipeline);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 6);
        vkCmdDispatch(computeLUTsCommandBuffer, 1, 32, 32);
        vkCmdWriteTimestamp(computeLUTsCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            perFrameData[i].querryPool, 7);
        #pragma endregion AEPerspectiveLUT

        vkCmdPipelineBarrier(
            computeLUTsCommandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1,
            &prevComputeWorkFinished, 
            0, nullptr,
            0, nullptr
        );

        vkEndCommandBuffer(computeLUTsCommandBuffer);
        #pragma endregion LUTs

        #pragma region RenderSky
        VkCommandBuffer renderSkyCommandBuffer = 
            findInMap(perFrameData[i].commandBuffers,"RenderSky");

        VkCommandBufferBeginInfo renderSkyCommandBufferBI {};
        renderSkyCommandBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if(vkBeginCommandBuffer(renderSkyCommandBuffer, &renderSkyCommandBufferBI) 
            != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::BUILD_COMPUTE_COMMAND_BUFFER::\
                Failed begin Render Sky graphics command buffer");
        }

        /* Terrain render into backbuffer */
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = hdrBackbufferPass;
        renderPassInfo.framebuffer = findInMap(perFrameData[i].framebuffers, "Offscreen");
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = vSwapChain->swapChainExtent;

        std::array<VkClearValue, 3> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        /* 1 in the depth buffer lies at the far view plane */
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /* =============================================== FIRST SUBPASS =============================================== */
        vkCmdBeginRenderPass(renderSkyCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            terrainPassPipeline->pipeline);

        std::vector<VkDescriptorSet> terrainDescriptorSets = {
            findInMap(perFrameData[i].descriptorSets,"CommonUBO"),
            findInMap(perFrameData[i].descriptorSets,"SkyConstantUBO"),
            findInMap(frameSharedDS,"TerrainTextures"),
            findInMap(perFrameData[i].descriptorSets,"TransmittanceLUT"),
        };
        vkCmdBindDescriptorSets(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            terrainPassPipeline->layout, 0, 4, terrainDescriptorSets.data(), 0, 0);

        VkDeviceSize offsets [] = {0};
        vkCmdBindVertexBuffers(renderSkyCommandBuffer, 0, 1, &(vertexBuffer.get()->buffer), offsets);
        vkCmdBindIndexBuffer(renderSkyCommandBuffer,indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

        /* TODO: Replace hardcoded num of indices */
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 8);
        vkCmdDrawIndexed(renderSkyCommandBuffer, 2999 * 2999 * 6, 1, 0, 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 9);

        /* =============================================== SECOND SUBPASS =============================================== */
        vkCmdNextSubpass(renderSkyCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            farSkyPassPipeline->pipeline);

        std::vector<VkDescriptorSet> skyDescriptorSets = { 
            findInMap(perFrameData[i].descriptorSets,"CommonUBO"),
            findInMap(perFrameData[i].descriptorSets,"SkyConstantUBO"),
            findInMap(perFrameData[i].descriptorSets,"SkyViewLUT"),
            findInMap(perFrameData[i].descriptorSets,"DepthOne")
        };
        vkCmdBindDescriptorSets(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            farSkyPassPipeline->layout, 0, 4, skyDescriptorSets.data(), 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 10);
        vkCmdDraw(renderSkyCommandBuffer, 3, 1, 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 11);

        /* =============================================== THIRD SUBPASS =============================================== */
        vkCmdNextSubpass(renderSkyCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            cloudsPassPipeline->pipeline);

        std::vector<VkDescriptorSet> cloudsDescriptorSets = { 
            findInMap(perFrameData[i].descriptorSets,"CommonUBO"),
            findInMap(perFrameData[i].descriptorSets,"SkyConstantUBO"),
            findInMap(perFrameData[i].descriptorSets,"CloudsParamsUBO"),
            findInMap(perFrameData[i].descriptorSets,"DepthOne"),
            findInMap(frameSharedDS,"WorleyNoise"),
            findInMap(perFrameData[i].descriptorSets,"TransmittanceLUT"),
        };
        vkCmdBindDescriptorSets(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            cloudsPassPipeline->layout, 0, 6, cloudsDescriptorSets.data(), 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 12);
        vkCmdDraw(renderSkyCommandBuffer, 3, 1, 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 13);

        /* =============================================== FOURTH SUBPASS =============================================== */
        vkCmdNextSubpass(renderSkyCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            aePerspectivePassPipeline->pipeline);

        std::vector<VkDescriptorSet> aePerspectiveDescriptorSets = {
            findInMap(perFrameData[i].descriptorSets,"CommonUBO"),
            findInMap(perFrameData[i].descriptorSets,"SkyConstantUBO"),
            findInMap(perFrameData[i].descriptorSets,"DepthTwo"),
            findInMap(perFrameData[i].descriptorSets,"AEPerspectiveLUT"),
        };
        vkCmdBindDescriptorSets(renderSkyCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            aePerspectivePassPipeline->layout, 0, 4, aePerspectiveDescriptorSets.data(), 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 14);
        vkCmdDraw(renderSkyCommandBuffer, 3, 1, 0, 0);
        vkCmdWriteTimestamp(renderSkyCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 15);

        vkCmdEndRenderPass(renderSkyCommandBuffer);
        vkEndCommandBuffer(renderSkyCommandBuffer);
        #pragma endregion RenderSky

        #pragma region postProcess
        VkCommandBuffer postProcessCommandBuffer = 
            findInMap(perFrameData[i].commandBuffers,"PostProcess");
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
    
        if (vkBeginCommandBuffer(postProcessCommandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_COMMAND_BUFFERS::\
                Failed to begin recording command buffer");
        }

        #pragma region histogramComputation

        vkCmdBindPipeline(postProcessCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            histogramPipeline->pipeline);
        std::array<VkDescriptorSet, 3> histogramDescriptorSets = {
            findInMap(perFrameData[i].descriptorSets,"HDRBackbuffer"),
            findInMap(perFrameData[i].descriptorSets,"HistogramSSBO"),
            findInMap(perFrameData[i].descriptorSets,"PostProcessUBO"),
        };
        vkCmdBindDescriptorSets(postProcessCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            histogramPipeline->layout, 0, 3, histogramDescriptorSets.data(), 0, nullptr);

        vkCmdWriteTimestamp(postProcessCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 16);
        vkCmdDispatch(postProcessCommandBuffer, vSwapChain->swapChainExtent.width/16, vSwapChain->swapChainExtent.height/16, 1);
        vkCmdWriteTimestamp(postProcessCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 17);
        #pragma endregion histogramComputation

        VkMemoryBarrier prevHistogramComputeWorkFinished = {};
        prevHistogramComputeWorkFinished.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        prevHistogramComputeWorkFinished.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        prevHistogramComputeWorkFinished.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            postProcessCommandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1,
            &prevHistogramComputeWorkFinished, 
            0, nullptr,
            0, nullptr
        );

        #pragma region sumHistogramComputation

        vkCmdBindPipeline(postProcessCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            sumHistogramPipeline->pipeline);
        std::array<VkDescriptorSet, 3> sumHistogramDescriptorSets = {
            findInMap(perFrameData[i].descriptorSets,"AvgLumSSBO"),
            findInMap(perFrameData[i].descriptorSets,"HistogramSSBO"),
            findInMap(perFrameData[i].descriptorSets,"PostProcessUBO"),
        };
        vkCmdBindDescriptorSets(postProcessCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
            sumHistogramPipeline->layout, 0, 3, sumHistogramDescriptorSets.data(), 0, nullptr);

        vkCmdWriteTimestamp(postProcessCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 18);
        vkCmdDispatch(postProcessCommandBuffer, 1, 1, 1);
        vkCmdWriteTimestamp(postProcessCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 19);

        vkCmdPipelineBarrier(
            postProcessCommandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            0, 1,
            &prevHistogramComputeWorkFinished, 
            0, nullptr,
            0, nullptr
        );
        #pragma endregion sumHistogramComputation

        /* Starting a render pass */
        VkRenderPassBeginInfo postProcessRenderPassInfo{};
        postProcessRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        postProcessRenderPassInfo.renderPass = renderPass;
        postProcessRenderPassInfo.framebuffer = findInMap(perFrameData[i].framebuffers, "FinalPass");
        postProcessRenderPassInfo.renderArea.offset = {0, 0};
        postProcessRenderPassInfo.renderArea.extent = vSwapChain->swapChainExtent;
    
        /* NOTE: The order of clear values should be identical to the order
                 of attachments */
        std::array<VkClearValue, 1> postProcessClearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    
        postProcessRenderPassInfo.clearValueCount = static_cast<uint32_t>(postProcessClearValues.size());
        postProcessRenderPassInfo.pClearValues = postProcessClearValues.data();

        vkCmdBeginRenderPass(postProcessCommandBuffer, &postProcessRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(postProcessCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            finalPassPipeline->pipeline);

        std::array<VkDescriptorSet, 3> descriptorSets_= {
            findInMap(perFrameData[i].descriptorSets,"HDRBackbuffer"),
            findInMap(perFrameData[i].descriptorSets,"AvgLumSSBO"),
            findInMap(perFrameData[i].descriptorSets,"PostProcessUBO"),
        };
        vkCmdBindDescriptorSets(postProcessCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            finalPassPipeline->layout, 0, 3, descriptorSets_.data(), 0, nullptr);

        vkCmdWriteTimestamp(postProcessCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 20);
        vkCmdDraw(postProcessCommandBuffer, 3, 1, 0, 0);
        vkCmdWriteTimestamp(postProcessCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            perFrameData[i].querryPool, 21);
        vkCmdEndRenderPass(postProcessCommandBuffer);
    
        if (vkEndCommandBuffer(postProcessCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_COMMAND_BUFFERS::Failed to record command buffer");
        }
        #pragma endregion postProcess
    }
}

void Renderer::createSyncObjects()
{ 
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    /* create the fence in signaled state by default so we don't wait forever in draw*/
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(vDevice->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vDevice->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vDevice->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("RENDERER::CREATE_SYNC_OBJECTS::Failed to create synchronization objects \
                                              for a frame");
        }
    }
}

static auto timeLastFrame = 0.0;
void Renderer::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float,std::chrono::seconds::period>
        (currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.model = glm::scale(ubo.model, glm::vec3(1000, 1000, 1000.0));
    ubo.model = glm::translate(ubo.model, glm::vec3(-0.5f, -0.5f, -0.0f));

    float aspectRatio = float(vSwapChain->swapChainExtent.width) / 
        float(vSwapChain->swapChainExtent.height);
    ubo.proj = glm::perspective(glm::radians(50.0f), aspectRatio, 0.1f, 20000.0f);
    /* GLM is using OpenGL standard where Y coordinate of the clip coordinates is inverted */
    ubo.proj[1][1] *= -1;

    ubo.view = camera->getViewMatrix();
    ubo.lHviewProj = ubo.proj * camera->getViewMatrix(true);
    ubo.time = time;
    void *data;
    vkMapMemory(vDevice->device, findInMap(perFrameData[currentImage].buffers, "CommonUBO")->bufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vDevice->device, findInMap(perFrameData[currentImage].buffers, "CommonUBO")->bufferMemory);


    atmoParamsBuffer.cameraPosition = camera->getPos();
    atmoParamsBuffer.sunDirection = glm::vec3(
        glm::cos(glm::radians(atmoParamsBuffer.sunPhiAngle)) * glm::sin(glm::radians(atmoParamsBuffer.sunThetaAngle)),
        glm::sin(glm::radians(atmoParamsBuffer.sunPhiAngle)) * glm::sin(glm::radians(atmoParamsBuffer.sunThetaAngle)),
        glm::cos(glm::radians(atmoParamsBuffer.sunThetaAngle))
    );

    vkMapMemory(vDevice->device, 
        findInMap(perFrameData[currentImage].buffers, "SkyConstantUBO")->bufferMemory, 0, 
        sizeof(AtmosphereParametersBuffer), 0, &data);
    memcpy(data, &atmoParamsBuffer, sizeof(AtmosphereParametersBuffer));
    vkUnmapMemory(vDevice->device, 
        findInMap(perFrameData[currentImage].buffers, "SkyConstantUBO")->bufferMemory);

    vkMapMemory(vDevice->device, 
        findInMap(perFrameData[currentImage].buffers, "CloudsParamsUBO")->bufferMemory, 0, 
        sizeof(CloudsParametersBuffer), 0, &data);
    memcpy(data, &cloudsParamsBuffer, sizeof(CloudsParametersBuffer));
    vkUnmapMemory(vDevice->device,
        findInMap(perFrameData[currentImage].buffers, "CloudsParamsUBO")->bufferMemory);

    float timeThisFrame = glfwGetTime();
    /* Cap this to 0.2 to not cause issues due to long render times of first frames */
    postProcessParamsBuffer.timeDelta = glm::min(timeThisFrame - timeLastFrame, 0.020); 
    timeLastFrame = timeThisFrame;
    if(postProcessParamsBuffer.minimumLuminance > postProcessParamsBuffer.maximumLuminance)
    {
        postProcessParamsBuffer.minimumLuminance = postProcessParamsBuffer.maximumLuminance;
    }
    vkMapMemory(vDevice->device, 
        findInMap(perFrameData[currentImage].buffers, "PostProcessUBO")->bufferMemory, 0, 
        sizeof(PostProcessParamsBuffer), 0, &data);
    memcpy(data, &postProcessParamsBuffer, sizeof(PostProcessParamsBuffer));
    vkUnmapMemory(vDevice->device, 
        findInMap(perFrameData[currentImage].buffers, "PostProcessUBO")->bufferMemory);
}

void Renderer::cleanupSwapchain()
{
    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        for(auto& image : perFrameData[i].images)
        {
            image.second.reset();
        }
        for(auto& framebuffer : perFrameData[i].framebuffers)
        {
            vkDestroyFramebuffer(vDevice->device, framebuffer.second, nullptr);
        }
        for(auto& commandBuffer : perFrameData[i].commandBuffers)
        {
            vkFreeCommandBuffers(vDevice->device, vDevice->graphicsCommandPool, 1, &commandBuffer.second);
        }
    }

    finalPassPipeline.reset();
    terrainPassPipeline.reset();
    cloudsPassPipeline.reset();
    farSkyPassPipeline.reset();
    aePerspectivePassPipeline.reset();
    transmittanceLUTPipeline.reset();
    multiscatteringLUTPipeline.reset();
    skyViewLUTPipeline.reset();
    AEPerspectiveLUTPipeline.reset();
    histogramPipeline.reset();
    sumHistogramPipeline.reset();

    vkDestroyRenderPass(vDevice->device, renderPass, nullptr); 
    vkDestroyRenderPass(vDevice->device, hdrBackbufferPass, nullptr); 
    vkDestroyDescriptorPool(vDevice->device, descriptorPool, nullptr);

    vSwapChain.reset();
}

void Renderer::recreateSwapChain()
{
    /* Hande window minimization -> this results in frame buffer size of 0 */
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
 
    /* Make sure to not touch resources that are still in use*/
    vkDeviceWaitIdle(vDevice->device);
    cleanupSwapchain();
    vSwapChain = std::make_unique<VulkanSwapChain>(vDevice, surface, width, height);

    postProcessParamsBuffer.texDimensions = 
        glm::vec2(vSwapChain->swapChainExtent.width, vSwapChain->swapChainExtent.height);
    createAttachments();
    createRenderPass();
    createPipelines();
    prepareTextureTargets(256, 64, VK_FORMAT_R16G16B16A16_SFLOAT);
    createFramebuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

void Renderer::prepareTextureTargets(uint32_t width, uint32_t height, VkFormat format)
{
    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        /* Transmittance LUT */
        perFrameData[i].images["TransmittanceLUT"] = std::make_unique<VulkanImage>(vDevice, width, height, 1,
            VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

        findInMap(perFrameData[i].images,"TransmittanceLUT")->TransitionImageLayout(format, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL, 1);

        /* Multiscattering LUT  */
        perFrameData[i].images["MultiscatteringLUT"] = std::make_unique<VulkanImage>(vDevice, 32, 32, 1,
            VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

        findInMap(perFrameData[i].images,"MultiscatteringLUT")->TransitionImageLayout(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL, 1);

        /* SkyView LUT */
        perFrameData[i].images["SkyViewLUT"] = std::make_unique<VulkanImage>(vDevice, 192, 128, 1,
            VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

        findInMap(perFrameData[i].images,"SkyViewLUT")->TransitionImageLayout(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL, 1);

        /* AEPerspctive LUT */
        perFrameData[i].images["AEPerspectiveLUT"] = std::make_unique<VulkanImage>(vDevice, 32, 32, 1,
            VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 32);

        findInMap(perFrameData[i].images,"AEPerspectiveLUT")->TransitionImageLayout(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL, 1);
    }

    VkSamplerCreateInfo terrainTexturesSamplerCI{};
    terrainTexturesSamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    terrainTexturesSamplerCI.magFilter = VK_FILTER_LINEAR;
    terrainTexturesSamplerCI.minFilter = VK_FILTER_LINEAR;
    terrainTexturesSamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    terrainTexturesSamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    terrainTexturesSamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    terrainTexturesSamplerCI.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties terrainTexturesProperties {};
    vkGetPhysicalDeviceProperties(vDevice->physicalDevice, &terrainTexturesProperties);
    terrainTexturesSamplerCI.maxAnisotropy = terrainTexturesProperties.limits.maxSamplerAnisotropy;
    terrainTexturesSamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    terrainTexturesSamplerCI.unnormalizedCoordinates = VK_FALSE;
    terrainTexturesSamplerCI.compareEnable = VK_FALSE;
    terrainTexturesSamplerCI.compareOp = VK_COMPARE_OP_NEVER;
    terrainTexturesSamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    terrainTexturesSamplerCI.mipLodBias = 0.0f;
    terrainTexturesSamplerCI.minLod = 0.0f;
    terrainTexturesSamplerCI.maxLod = static_cast<float>(findInMap(frameSharedImages,"TerrainDiffuseImage")->mipLevels);

    if (vkCreateSampler(vDevice->device, &terrainTexturesSamplerCI, nullptr, &terrainTexturesSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_TEXTURE_SAMPLER::Failed to create terrain texture sampler");
    }

    VkSamplerCreateInfo skyViewLUTSamplerInfo{};
    skyViewLUTSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    skyViewLUTSamplerInfo.magFilter = VK_FILTER_LINEAR;
    skyViewLUTSamplerInfo.minFilter = VK_FILTER_LINEAR;
    skyViewLUTSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    skyViewLUTSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    skyViewLUTSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    skyViewLUTSamplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties skyViewProperties {};
    vkGetPhysicalDeviceProperties(vDevice->physicalDevice, &skyViewProperties);
    skyViewLUTSamplerInfo.maxAnisotropy = skyViewProperties.limits.maxSamplerAnisotropy;
    skyViewLUTSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    skyViewLUTSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    skyViewLUTSamplerInfo.compareEnable = VK_FALSE;
    skyViewLUTSamplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    skyViewLUTSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    skyViewLUTSamplerInfo.mipLodBias = 0.0f;
    skyViewLUTSamplerInfo.minLod = 0.0f;
    skyViewLUTSamplerInfo.maxLod = static_cast<float>(findInMap(perFrameData[0].images,"SkyViewLUT")->mipLevels);

    if (vkCreateSampler(vDevice->device, &skyViewLUTSamplerInfo, nullptr, &skyViewLUTSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_TEXTURE_SAMPLER::Failed to create skyView texture sampler");
    }

    VkSamplerCreateInfo cloudsSamplerInfo{};
    cloudsSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    cloudsSamplerInfo.magFilter = VK_FILTER_LINEAR;
    cloudsSamplerInfo.minFilter = VK_FILTER_LINEAR;
    cloudsSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    cloudsSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    cloudsSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    cloudsSamplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties cloudsProperties {};
    vkGetPhysicalDeviceProperties(vDevice->physicalDevice, &cloudsProperties);
    cloudsSamplerInfo.maxAnisotropy = skyViewProperties.limits.maxSamplerAnisotropy;
    cloudsSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    cloudsSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    cloudsSamplerInfo.compareEnable = VK_FALSE;
    cloudsSamplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    cloudsSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    cloudsSamplerInfo.mipLodBias = 0.0f;
    cloudsSamplerInfo.minLod = 0.0f;
    cloudsSamplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(vDevice->device, &cloudsSamplerInfo, nullptr, &cloudsSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_TEXTURE_SAMPLER::Failed to create skyView texture sampler");
    }

    VkSamplerCreateInfo depthSamplerInfo{};
    depthSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    depthSamplerInfo.magFilter = VK_FILTER_LINEAR;
    depthSamplerInfo.minFilter = VK_FILTER_LINEAR;
    depthSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    depthSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    depthSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    depthSamplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties depthProperties {};
    vkGetPhysicalDeviceProperties(vDevice->physicalDevice, &depthProperties);
    depthSamplerInfo.maxAnisotropy = 1.0f;
    depthSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    depthSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    depthSamplerInfo.compareEnable = VK_FALSE;
    depthSamplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    depthSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    depthSamplerInfo.mipLodBias = 0.0f;
    depthSamplerInfo.minLod = 0.0f;
    depthSamplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(vDevice->device, &depthSamplerInfo, nullptr, &depthTextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_TEXTURE_SAMPLER::Failed to create depth texture sampler");
    }

    VkSamplerCreateInfo AEPerspectiveLUTSamplerInfo{};
    AEPerspectiveLUTSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    AEPerspectiveLUTSamplerInfo.magFilter = VK_FILTER_LINEAR;
    AEPerspectiveLUTSamplerInfo.minFilter = VK_FILTER_LINEAR;
    AEPerspectiveLUTSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    AEPerspectiveLUTSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    AEPerspectiveLUTSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    AEPerspectiveLUTSamplerInfo.anisotropyEnable = VK_TRUE;

    AEPerspectiveLUTSamplerInfo.maxAnisotropy = skyViewProperties.limits.maxSamplerAnisotropy;
    AEPerspectiveLUTSamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    AEPerspectiveLUTSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    AEPerspectiveLUTSamplerInfo.compareEnable = VK_FALSE;
    AEPerspectiveLUTSamplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    AEPerspectiveLUTSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    AEPerspectiveLUTSamplerInfo.mipLodBias = 0.0f;
    AEPerspectiveLUTSamplerInfo.minLod = 0.0f;
    AEPerspectiveLUTSamplerInfo.maxLod = 1;

    if (vkCreateSampler(vDevice->device, &AEPerspectiveLUTSamplerInfo, nullptr, &AEPerspectiveSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("APP::CREATE_TEXTURE_SAMPLER::Failed to create AEPerspective texture sampler");
    }
}

void Renderer::createComputeSyncObjects()
{

    VkSemaphoreCreateInfo graphicsSemaphoreCreateInfo {};
    graphicsSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(vkCreateSemaphore(vDevice->device, &graphicsSemaphoreCreateInfo, nullptr,
        &graphicsSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_COMPUTE_SYNC_OBJECTS::Failed \
            to create compute sychroniztion objects");
    }

    VkSemaphoreCreateInfo skyViewComputeSemaphoreInfo {};
    skyViewComputeSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(vkCreateSemaphore(vDevice->device, &skyViewComputeSemaphoreInfo, nullptr, 
        &postProcessReadySemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::CREATE_COMPUTE_SYNC_OBJECTS::Failed \
            to create compute sychroniztion objects");
    }
}

bool redrawNoise = true;
void Renderer::drawComputeFrame()
{
    uint32_t imageIndex;

    /* Wait for frame data to not be in use */
    vkWaitForFences(vDevice->device, 1, &inFlightFences[currentFrame],
        VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(vDevice->device, vSwapChain->swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    /* If Swap chain is out of date (due to window resize f.e.) call recreate swapchain*/
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        framebufferResized = false;
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("RENDERER::DRAW_FRAME::Failed to acquire swap chain image");
    }
    
    if (perFrameData[imageIndex].inFlightFence != VK_NULL_HANDLE)
    {
        vkWaitForFences(vDevice->device, 1, &perFrameData[imageIndex].inFlightFence,
            VK_TRUE, UINT64_MAX);
    }
    perFrameData[imageIndex].inFlightFence = inFlightFences[currentFrame];

    // Query timestamp results of the current image since they are guaranteed to already
    // have been written here
    vkGetQueryPoolResults(vDevice->device, perFrameData[imageIndex].querryPool,
        0, 22, 22*sizeof(uint64_t), perFrameData[imageIndex].timestamps.data(),
        0, VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_64_BIT);


    updateUniformBuffer(imageIndex);
    if(redrawNoise)
    {
        noise->generateNoise();
        detailNoise->generateNoise();
        redrawNoise = false;
    }

    VkSubmitInfo ComputeLUTsSI{};
    std::array<VkCommandBuffer, 2> commandBuffers = {
        findInMap(perFrameData[imageIndex].commandBuffers,"ComputeLUTs"), 
        findInMap(perFrameData[imageIndex].commandBuffers,"RenderSky"), 
    };

    ComputeLUTsSI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    ComputeLUTsSI.commandBufferCount = 2;
    ComputeLUTsSI.pCommandBuffers = commandBuffers.data();
    ComputeLUTsSI.waitSemaphoreCount = 0;
    ComputeLUTsSI.pWaitSemaphores = nullptr;
    ComputeLUTsSI.pWaitDstStageMask = VK_NULL_HANDLE;
    ComputeLUTsSI.signalSemaphoreCount = 1;
    ComputeLUTsSI.pSignalSemaphores = &postProcessReadySemaphore;

    if(vkQueueSubmit(vDevice->graphicsQueue, 1, &ComputeLUTsSI, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::DRAW_FRAME::Failed to submit multiscattering compute queue");
    }

    VkPipelineStageFlags graphicsWaitStageMasks[] = { 
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
    };

    VkSemaphore graphicsWaitSemaphores[] = {
        imageAvailableSemaphores[currentFrame],
        postProcessReadySemaphore
    };

    glm::vec2 extent = glm::vec2(vSwapChain->swapChainExtent.width,vSwapChain->swapChainExtent.height);
    std::array<VkCommandBuffer, 2> submitCommandBuffers = {
        findInMap(perFrameData[imageIndex].commandBuffers, "PostProcess"), 
        imguiImpl->PrepareNewFrame(
            imageIndex,
            findInMap(perFrameData[imageIndex].framebuffers, "ImGui"), camera, 
            postProcessParamsBuffer, atmoParamsBuffer, cloudsParamsBuffer,
            perFrameData[imageIndex].timestamps, extent)
    };

    //submit graphics commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
    submitInfo.pCommandBuffers = submitCommandBuffers.data();
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = graphicsWaitSemaphores;
    submitInfo.pWaitDstStageMask = graphicsWaitStageMasks;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    /* When post process and ImGUI command buffers are finished signal frameData fence
       so that another frame can be submitted */
    vkResetFences(vDevice->device, 1, &inFlightFences[currentFrame]);
    if (vkQueueSubmit(vDevice->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("RENDERER::DRAW_FRAME::Failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vSwapChain->swapChain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(vDevice->presentQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// Timestamps
void Renderer::createQuerryPool()
{
    for(int i = 0; i < vSwapChain->imageCount; i++)
    {
        VkQueryPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.pNext = nullptr; // Optional
        createInfo.flags = 0; // Reserved for future use, must be 0!

        createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount = 30;

        VkResult result = vkCreateQueryPool(vDevice->device, &createInfo, nullptr, &perFrameData[i].querryPool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create time query pool!");
        }
    }
}