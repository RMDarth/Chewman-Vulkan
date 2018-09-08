// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <iostream>
#include "VulkanInstance.h"
#include "VulkanException.h"
#include "VulkanMesh.h"
#include "VulkanMaterial.h"

namespace SVE
{
namespace
{
const char *const VK_LAYER_LUNARG_STANDARD_VALIDATION = "VK_LAYER_LUNARG_standard_validation";

bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool layerFound = false;

    for (const auto &layerProperties : availableLayers)
    {
        if (strcmp(VK_LAYER_LUNARG_STANDARD_VALIDATION, layerProperties.layerName) == 0)
        {
            layerFound = true;
            break;
        }
    }

    return layerFound;
}

VkResult CreateDebugReportCallbackEXT(
        VkInstance instance,
        const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugReportCallbackEXT *pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugReportCallbackEXT(VkInstance instance,
                                   VkDebugReportCallbackEXT callback,
                                   const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                            "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT /*flags*/,
        VkDebugReportObjectTypeEXT /*objType*/,
        uint64_t /*obj*/,
        size_t /*location*/,
        int32_t /*code*/,
        const char* /*layerPrefix*/,
        const char* msg,
        void* /*userData*/)
{
    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

} // anon namespace

VulkanInstance::VulkanInstance(SDL_Window* window, EngineSettings settings)
    : _engineSettings(settings)
    , _window(window)
    , _vulkanUtils(this)

{
    createInstance();
    createDebugCallback();
    createDevice();
    createSurface();
    createSurfaceParameters();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createCommandPool();
    createMSAABuffer();
    createDepthBuffer();
    createFramebuffers();
    createSyncPrimitives();
}

VulkanInstance::~VulkanInstance()
{
    deleteSyncPrimitives();
    deleteFramebuffers();
    deleteDepthBuffer();
    deleteMSAABuffer();
    deleteCommandPool();
    deleteRenderPass();
    deleteImageViews();
    deleteSwapchain();
    deleteSurfaceParameters();
    deleteSurface();
    deleteDevice();
    deleteDebugCallback();
    deleteInstance();
}

const VulkanUtils& VulkanInstance::getVulkanUtils() const
{
    return _vulkanUtils;
}


VkInstance VulkanInstance::getInstance() const
{
    return _instance;
}

VkPhysicalDevice VulkanInstance::getGPU() const
{
    return _gpu;
}

VkDevice VulkanInstance::getLogicalDevice() const
{
    return _device;
}

VkCommandPool VulkanInstance::getCommandPool() const
{
    return _commandPool;
}

VkRenderPass VulkanInstance::getRenderPass() const
{
    return _renderPass;
}

VkExtent2D VulkanInstance::getExtent() const
{
    return _extent;
}

VkQueue VulkanInstance::getGraphicsQueue() const
{
    return _queue;
}

size_t VulkanInstance::getSwapchainSize() const
{
    return _swapchainImages.size();
}

VkFramebuffer VulkanInstance::getFramebuffer(size_t index) const
{
    return _swapchainFramebuffers[index];
}

VkSemaphore* VulkanInstance::getImageAvailableSemaphore()
{
    return &_imageAvailableSemaphore;
}

VkPipeline VulkanInstance::createPipeline(const VulkanMesh& vulkanMesh)
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = vulkanMesh.getMaterial()->createShaderStages();

    // Triangle data setup
    auto bindingDescriptions = vulkanMesh.getBindingDescription();
    auto attributeDescriptions = vulkanMesh.getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) _extent.width;
    viewport.height = (float) _extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _extent;

    VkPipelineViewportStateCreateInfo viewportCreateInfo{};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pViewports = &viewport;
    viewportCreateInfo.scissorCount = 1; // should be the same as viewport count
    viewportCreateInfo.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE; // clamp objects beyond near and far plane to the edges
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.lineWidth = 1.0f;
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE; // for altering depth (used in shadow mapping)

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = _msaaSamples;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    // Depth and stencil
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo {};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

    // Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo blendingCreateInfo{};
    blendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendingCreateInfo.logicOpEnable = VK_FALSE;
    blendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    blendingCreateInfo.attachmentCount = 1;
    blendingCreateInfo.pAttachments = &colorBlendAttachment;
    //blendingCreateInfo.blendConstants[0] = 0.0f;

    // Pipeline layout
    auto pipelineLayout = vulkanMesh.getMaterial()->getPipelineLayout();

    // Finally create pipeline
    VkPipeline pipeline;
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineCreateInfo.pColorBlendState = &blendingCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = _renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no deriving from other pipeline
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) !=
        VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Graphics Pipeline");
    }

    vulkanMesh.getMaterial()->freeShaderModules();

    return pipeline;
}

void VulkanInstance::waitAvailableFramebuffer()
{
    /*vkWaitForFences(_device,
                    _inFlightFences.size(),
                    _inFlightFences.data(),
                    VK_FALSE,
                    std::numeric_limits<uint64_t>::max());*/

    // acquire image
    auto result = vkAcquireNextImageKHR(_device, _swapchain, std::numeric_limits<uint64_t>::max(),
                                        _imageAvailableSemaphore, VK_NULL_HANDLE, &_currentImageIndex);

    vkWaitForFences(_device,
                    1,
                    &_inFlightFences[_currentImageIndex],
                    VK_TRUE,
                    std::numeric_limits<uint64_t>::max());

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        //resizeWindow();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw VulkanException("Can't acquire Vulkan Swapchain image");
    }
}

void VulkanInstance::submitCommands(const std::vector<SubmitInfo>& submitList) const
{
    std::vector<VkSubmitInfo> vkSubmitList;
    std::vector<VkSemaphore> vkSemaphoresList;
    for (auto& info : submitList)
    {
        auto si = info.getVulkanSubmitInfo();
        vkSemaphoresList.push_back(*si.pSignalSemaphores);
        vkSubmitList.emplace_back(std::move(si));
    }

    vkResetFences(_device, 1, &_inFlightFences[_currentImageIndex]);
    if (vkQueueSubmit(_queue, vkSubmitList.size(), vkSubmitList.data(), _inFlightFences[_currentImageIndex]) != VK_SUCCESS)
    {
        throw VulkanException("Can't submit Vulkan command buffers to queue");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = vkSemaphoresList.size();
    presentInfo.pWaitSemaphores = vkSemaphoresList.data();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.pImageIndices = &_currentImageIndex;
    presentInfo.pResults = nullptr;

    auto result = vkQueuePresentKHR(_queue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        //resizeWindow();
    } else if (result != VK_SUCCESS) {
        throw VulkanException("Can't present swapchain image");
    }
}

uint32_t VulkanInstance::getCurrentImageIndex() const
{
    return _currentImageIndex;
}

void VulkanInstance::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = _engineSettings.applicationName;

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    std::vector<const char *> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME
    };
    addPlatformSpecificExtensions(extensions);

    const std::vector<const char *> validationLayers = {
            VK_LAYER_LUNARG_STANDARD_VALIDATION
    };
    if (_engineSettings.useValidation)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    if (_engineSettings.useValidation)
    {
        if (!checkValidationLayerSupport())
        {
            throw VulkanException("Vulkan standard validation layer not supported");
        }
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&instanceInfo, nullptr, &_instance) != VK_SUCCESS)
    {
        throw VulkanException("Vulkan Instance not created");
    }
}

void VulkanInstance::deleteInstance()
{
    vkDestroyInstance(_instance, nullptr);
}

void VulkanInstance::createDevice()
{
    {
        uint32_t gpuCount;
        vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr); // query device count
        if (gpuCount == 0)
        {
            throw VulkanException("Can't find Vulkan enabled device");
        }
        std::vector<VkPhysicalDevice> gpuList(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuList.data());
        _gpu = gpuList[getGPUIndex(gpuList)];

        vkGetPhysicalDeviceProperties(_gpu, &_gpuProps);
        _msaaSamples = getMSAALevelsValue(_engineSettings.MSAALevel);
    }

    {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &queueFamilyCount, queueFamilyProps.data());

        const auto graphicsQueueIter = std::find_if(queueFamilyProps.begin(), queueFamilyProps.end(),
                                                    [](const VkQueueFamilyProperties &prop)
                                                    {
                                                        return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                                                    });

        if (graphicsQueueIter != queueFamilyProps.end())
        {
            _queueIndex = static_cast<uint32_t>(std::distance(queueFamilyProps.begin(), graphicsQueueIter));
        }
        else
        {
            throw VulkanException("GPU doesn't support graphics output");
        }
    }

    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.queueFamilyIndex = _queueIndex;
    deviceQueueCreateInfo.pQueuePriorities = priorities;

    // TODO: Add device extension support check (mb when displaying list of supported GPUs)
    std::vector<const char *> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // TODO: move filtering method to EngineSettings
    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = extensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateDevice(_gpu, &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS)
    {
        throw VulkanException("Cannot create Vulkan device");
    }

    vkGetDeviceQueue(_device, _queueIndex, 0, &_queue);
}

void VulkanInstance::deleteDevice()
{
    vkDestroyDevice(_device, nullptr);
    _device = nullptr;
}


void VulkanInstance::createDebugCallback()
{
    if (!_engineSettings.useValidation)
        return;

    VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
    debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT; // | VK_DEBUG_REPORT_INFORMATION_BIT_EXT for verbose output
    debugCallbackCreateInfo.pfnCallback = debugCallback;

    if (CreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugCallbackHandle) != VK_SUCCESS)
    {
        throw VulkanException("Vulkan failed to set up debug callback!");
    }
}

void VulkanInstance::deleteDebugCallback()
{
    if (_engineSettings.useValidation)
    {
        DestroyDebugReportCallbackEXT(_instance, _debugCallbackHandle, nullptr);
        _debugCallbackHandle = VK_NULL_HANDLE;
    }
}

void VulkanInstance::createSurface()
{
    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_surface))
    {
        throw VulkanException("Cannot create Vulkan window surface");
    }
}

void VulkanInstance::deleteSurface()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

void VulkanInstance::createSurfaceParameters()
{
    // check present support
    VkBool32 presentSupported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, _queueIndex, _surface, &presentSupported);
    if (!presentSupported)
    {
        // TODO: add support for separate queue
        throw VulkanException("Different queues are not currently supported");
    }

    // get surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpu, _surface, &_surfaceCapabilities);

    // get surface format
    {
        uint32_t formatsCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, _surface, &formatsCount, nullptr);
        if (!formatsCount)
        {
            throw VulkanException("Vulkan surface has no supported formats");
        }
        std::vector<VkSurfaceFormatKHR> formatList(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, _surface, &formatsCount, formatList.data());
        // TODO: rank formats and pick best (usually VK_FORMAT_B8G8R8A8_UNORM is used)
        _surfaceFormat = formatList.front();
        if (_surfaceFormat.format == VK_FORMAT_UNDEFINED)
        {
            _surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        }
    }

    // get present mode
    {
        uint32_t presentModesCount = 0;
        _presentMode = VK_PRESENT_MODE_FIFO_KHR;

        vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, _surface, &presentModesCount, nullptr);
        if (!presentModesCount)
        {
            throw VulkanException("Vulkan surface has no presentation modes");
        }
        std::vector<VkPresentModeKHR> presentModesList(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, _surface, &presentModesCount, presentModesList.data());

        // TODO: There could be issues with FIFO mode, maybe add possible swap to IMMEDIATE mode
        static VkPresentModeKHR presentModesMap[] = {
                VK_PRESENT_MODE_FIFO_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_IMMEDIATE_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR
        };
        auto expectedPresentMode = presentModesMap[static_cast<uint8_t>(_engineSettings.presentMode)];
        if (std::find(presentModesList.begin(), presentModesList.end(), expectedPresentMode) != presentModesList.end())
        {
            _presentMode = expectedPresentMode;
        }
    }

    // get swapchain extent
    SDL_Vulkan_GetDrawableSize(_window, &_windowWidth, &_windowHeight);
    if (_surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        _extent = _surfaceCapabilities.currentExtent;
    }
    else
    {
        _extent.width = std::max(_surfaceCapabilities.minImageExtent.width,
                                 std::min(_surfaceCapabilities.maxImageExtent.width, (uint32_t) _windowWidth));
        _extent.height = std::max(_surfaceCapabilities.minImageExtent.height,
                                  std::min(_surfaceCapabilities.maxImageExtent.height, (uint32_t) _windowHeight));
    };
}

void VulkanInstance::deleteSurfaceParameters()
{

}


void VulkanInstance::createSwapchain()
{
    uint32_t imageCount = _surfaceCapabilities.minImageCount + 1;
    if (_surfaceCapabilities.maxImageCount > 0)
    {
        imageCount = std::min(_surfaceCapabilities.maxImageCount, imageCount);
    }

    // Create swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = _surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = _surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = _surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = _extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // TODO: if we have different queues for presenting and drawing, we should specify them and set correct sharing
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    swapchainCreateInfo.preTransform = _surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // this is how alpha color should affect other windows
    swapchainCreateInfo.presentMode = _presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // used when recreating (when resizing)

    if (vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain) != VK_SUCCESS)
    {
        throw VulkanException("Failed to create Vulkan swapchain");
    }

    uint32_t realImagesCount;
    vkGetSwapchainImagesKHR(_device, _swapchain, &realImagesCount, nullptr);
    _swapchainImages.resize(realImagesCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &realImagesCount, _swapchainImages.data());
}

void VulkanInstance::deleteSwapchain()
{
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

void VulkanInstance::createImageViews()
{
    _swapchainImageViews.reserve(_swapchainImages.size());
    _swapchainImageViews.clear();
    for (auto &image : _swapchainImages)
    {
        _swapchainImageViews.emplace_back(_vulkanUtils.createImageView(image, _surfaceFormat.format, 1));
    }
}

void VulkanInstance::deleteImageViews()
{
    for (auto imageView : _swapchainImageViews)
    {
        vkDestroyImageView(_device, imageView, nullptr);
    }
}

void VulkanInstance::createRenderPass()
{
    VkAttachmentDescription colorAttachment {}; // color buffer attachment to render pass
    colorAttachment.format = _surfaceFormat.format;
    colorAttachment.samples = _msaaSamples; // for multisampling
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear every frame
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // should be STORE for rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // for use in MSAA( otherwise VK_IMAGE_LAYOUT_PRESENT_SRC_KHR needed)

    // resolve color attachment (after MSAA applied)
    VkAttachmentDescription colorAttachmentResolve {};
    colorAttachmentResolve.format = _surfaceFormat.format;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // to present in swapchain

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = _msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // this array correspond to fragment shader output layout
    subpass.pDepthStencilAttachment = &depthAttachmentRef; // only one depth attachment possible
    subpass.pResolveAttachments = &colorAttachmentResolveRef; // MSAA attachment

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // implicit subpass (pre-render pass)
    subpassDependency.dstSubpass = 0; // current subpass
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // waiting until swapchain finish reading from image (i.e. color attachment output stage)
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment, colorAttachmentResolve};

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 3;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan render pass");
    }
}

void VulkanInstance::deleteRenderPass()
{
    vkDestroyRenderPass(_device, _renderPass, nullptr);
}

void VulkanInstance::createCommandPool()
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = _queueIndex;

    if (vkCreateCommandPool(_device, &poolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Command Pool");
    }
}

void VulkanInstance::deleteCommandPool()
{
    vkDestroyCommandPool(_device, _commandPool, nullptr);
}

void VulkanInstance::createMSAABuffer()
{
    VkFormat colorFormat = _surfaceFormat.format;

    _vulkanUtils.createImage(
            _extent.width,
            _extent.height,
            1,
            _msaaSamples,
            colorFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _colorImage,
            _colorImageMemory);
    _colorImageView = _vulkanUtils.createImageView(_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    _vulkanUtils.transitionImageLayout(
            _colorImage,
            colorFormat,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            1);
}

void VulkanInstance::deleteMSAABuffer()
{
    vkDestroyImageView(_device, _colorImageView, nullptr);
    vkDestroyImage(_device, _colorImage, nullptr);
    vkFreeMemory(_device, _colorImageMemory, nullptr);
}

void VulkanInstance::createDepthBuffer()
{
    VkFormat depthFormat = findDepthFormat();
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    _vulkanUtils.createImage(
            _extent.width,
            _extent.height,
            1,
            _msaaSamples,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _depthImage,
            _depthImageMemory);
    _depthImageView = _vulkanUtils.createImageView(_depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

    _vulkanUtils.transitionImageLayout(
            _depthImage,
            depthFormat,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
            1,
            aspectFlags);
}

void VulkanInstance::deleteDepthBuffer()
{
    vkDestroyImageView(_device, _depthImageView, nullptr);
    vkDestroyImage(_device, _depthImage, nullptr);
    vkFreeMemory(_device, _depthImageMemory, nullptr);
}

void VulkanInstance::createFramebuffers()
{
    _swapchainFramebuffers.resize(_swapchainImageViews.size());

    for (size_t i = 0; i < _swapchainImageViews.size(); i++)
    {
        VkImageView attachments[] = {_colorImageView, _depthImageView, _swapchainImageViews[i]};

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = _renderPass;
        framebufferCreateInfo.attachmentCount = 3;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = _extent.width;
        framebufferCreateInfo.height = _extent.height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(_device, &framebufferCreateInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan Framebuffer");
        }
    }
}

void VulkanInstance::deleteFramebuffers()
{
    for (auto framebuffer : _swapchainFramebuffers)
    {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
}

void VulkanInstance::createSyncPrimitives()
{
    _inFlightFences.resize(_swapchainImages.size());

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan semaphore");
    }

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0u; i < _swapchainImages.size(); i++)
    {
        if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Vulkan fence");
        }
    }
}

void VulkanInstance::deleteSyncPrimitives()
{
    vkDestroySemaphore(_device, _imageAvailableSemaphore, nullptr);
    for (auto i = 0u; i < _swapchainImages.size(); i++)
    {
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }
}

void VulkanInstance::addPlatformSpecificExtensions(std::vector<const char *> &extensionsList)
{
    unsigned int count;
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &count, nullptr))
    {
        throw VulkanException("Can't get SDL Vulkan Instance extensions");
    }

    auto currentSize = extensionsList.size();
    extensionsList.resize(currentSize + count);
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &count, extensionsList.data() + currentSize))
    {
        throw VulkanException("Can't get SDL Vulkan Instance extensions");
    }
}

VkSampleCountFlagBits VulkanInstance::getMSAALevelsValue(int msaaLevels)
{
    VkSampleCountFlags counts;
    if (msaaLevels == EngineSettings::BEST_MSAA_AVAILABLE)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(_gpu, &physicalDeviceProperties);

        counts = std::min(
                physicalDeviceProperties.limits.framebufferColorSampleCounts,
                physicalDeviceProperties.limits.framebufferDepthSampleCounts);
    } else {
        counts = static_cast<VkSampleCountFlags>(msaaLevels);
    }

    if (counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

size_t VulkanInstance::getGPUIndex(std::vector<VkPhysicalDevice>& deviceList)
{
    if (_engineSettings.gpuIndex == EngineSettings::BEST_GPU_AVAILABLE)
    {
        size_t bestIndex = 0;
        size_t bestScore = 0;

        for (auto i = 0u; i < deviceList.size(); i++)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(deviceList[i], &deviceProperties);

            size_t score = 0;
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 1000;
            }
            score += deviceProperties.limits.maxImageDimension2D;

            if (score >= bestScore)
            {
                bestScore = score;
                bestIndex = i;
            }
        }

        return bestIndex;
    } else {
        return static_cast<size_t>(_engineSettings.gpuIndex);
    }
}

VkFormat VulkanInstance::findDepthFormat()
{
    return _vulkanUtils.findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

} // namespace SVE