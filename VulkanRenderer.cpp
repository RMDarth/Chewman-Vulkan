#include "VulkanRenderer.h"
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <chrono>
#include <SDL2/SDL_vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace
{
const char *const VK_LAYER_LUNARG_STANDARD_VALIDATION = "VK_LAYER_LUNARG_standard_validation";

std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("Can't open file " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

} // anon namespace


VkVertexInputBindingDescription VulkanRenderer::Vertex::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex or per instance

    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> VulkanRenderer::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

// Uniform Buffer Object
struct MatricesUBO
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

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

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
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
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char *layerPrefix,
        const char *msg,
        void *userData)
{
    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

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



VulkanRenderer::VulkanRenderer(SDL_Window *window, bool enableValidation)
        : _enableValidation(enableValidation), _window(window)
{
    createInstance();
    createDebugCallback();
    createDevice();
    createSurface();
    createSurfaceParameters();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createMSAABuffer();
    createDepthBuffer();
    createFramebuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel(); //
    createGeometryBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncPrimitives();
}

VulkanRenderer::~VulkanRenderer()
{
    deleteSyncPrimitives();
    deleteCommandBuffers();
    deleteDescriptorSets();
    deleteDescriptorPool();
    deleteUniformBuffers();
    unloadModel();
    deleteGeometryBuffers();
    deleteTextureSampler();
    deleteTextureImageView();
    deleteTextureImage();
    deleteFramebuffers();
    deleteDepthBuffer();
    deleteMSAABuffer();
    deleteCommandPool();
    deleteGraphicsPipeline();
    deleteDescriptorSetLayout();
    deleteRenderPass();
    deleteImageViews();
    deleteSwapchain();
    deleteSurfaceParameters();
    deleteSurface();
    deleteDevice();
    deleteDebugCallback();
    deleteInstance();
}

void VulkanRenderer::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = "Chewman";

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
    if (_enableValidation)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    if (_enableValidation)
    {
        if (!checkValidationLayerSupport())
        {
            throw std::runtime_error("Vulkan standard validation layer not supported");
        }
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&instanceInfo, nullptr, &_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan Instance not created");
    }
}

void VulkanRenderer::deleteInstance()
{
    vkDestroyInstance(_instance, nullptr);
    _instance = nullptr;
}

void VulkanRenderer::addPlatformSpecificExtensions(std::vector<const char *> &extensionsList)
{
    unsigned int count;
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &count, nullptr))
    {
        throw std::runtime_error("Can't get SDL Vulkan Instance extensions");
    }

    auto currentSize = extensionsList.size();
    extensionsList.resize(currentSize + count);
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &count, extensionsList.data() + currentSize))
    {
        throw std::runtime_error("Can't get SDL Vulkan Instance extensions");
    }
}

void VulkanRenderer::createDevice()
{
    {
        uint32_t gpuCount;
        vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr); // query device count
        if (gpuCount == 0)
        {
            throw std::runtime_error("Can't find Vulkan enabled device");
        }
        std::vector<VkPhysicalDevice> gpuList(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuList.data());
        // TODO: Implement selection of best or user-selected device
        _gpu = gpuList.front();

        vkGetPhysicalDeviceProperties(_gpu, &_gpuProps);
        _msaaSamples = getMaxUsableSampleCount();
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
            throw std::runtime_error("GPU doesn't support graphics output");
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
        throw std::runtime_error("Cannot create Vulkan device");
    }

    vkGetDeviceQueue(_device, _queueIndex, 0, &_queue);
}

void VulkanRenderer::deleteDevice()
{
    vkDestroyDevice(_device, nullptr);
    _device = nullptr;
}


void VulkanRenderer::createDebugCallback()
{
    if (!_enableValidation)
        return;

    VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo = {};
    debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT; // | VK_DEBUG_REPORT_INFORMATION_BIT_EXT for verbose output
    debugCallbackCreateInfo.pfnCallback = debugCallback;

    if (CreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugCallbackHandle) != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan failed to set up debug callback!");
    }
}

void VulkanRenderer::deleteDebugCallback()
{
    if (_enableValidation)
    {
        DestroyDebugReportCallbackEXT(_instance, _debugCallbackHandle, nullptr);
        _debugCallbackHandle = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::createSurface()
{
    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_surface))
    {
        throw std::runtime_error("Cannot create Vulkan window surface");
    }
}

void VulkanRenderer::deleteSurface()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

void VulkanRenderer::createSurfaceParameters()
{
    // check present support
    VkBool32 presentSupported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(_gpu, _queueIndex, _surface, &presentSupported);
    if (!presentSupported)
    {
        // TODO: add support for separate queue
        throw std::runtime_error("Different queues are not currently supported");
    }

    // get surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpu, _surface, &_surfaceCapabilities);

    // get surface format
    {
        uint32_t formatsCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_gpu, _surface, &formatsCount, nullptr);
        if (!formatsCount)
        {
            throw std::runtime_error("Vulkan surface has no supported formats");
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
            throw std::runtime_error("Vulkan surface has no presentation modes");
        }
        std::vector<VkPresentModeKHR> presentModesList(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_gpu, _surface, &presentModesCount, presentModesList.data());

        // TODO: There could be issues with FIFO mode, maybe add possible swap to IMMEDIATE mode
        /*if (std::find(presentModesList.begin(), presentModesList.end(), VK_PRESENT_MODE_MAILBOX_KHR) !=
            presentModesList.end())
        {
            _presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }*/
    }

    // get swapchain extent
    SDL_Vulkan_GetDrawableSize(_window, &_windowWidth, &_windowHeight);
    if (_surfaceCapabilities.currentExtent.width != UINT32_MAX)
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

void VulkanRenderer::deleteSurfaceParameters()
{

}


void VulkanRenderer::createSwapchain()
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
        throw std::runtime_error("Failed to create Vulkan swapchain");
    }

    uint32_t realImagesCount;
    vkGetSwapchainImagesKHR(_device, _swapchain, &realImagesCount, nullptr);
    _swapchainImages.resize(realImagesCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &realImagesCount, _swapchainImages.data());
}

void VulkanRenderer::deleteSwapchain()
{
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

void VulkanRenderer::createImageViews()
{
    _swapchainImageViews.reserve(_swapchainImages.size());
    _swapchainImageViews.clear();
    for (auto &image : _swapchainImages)
    {
        _swapchainImageViews.emplace_back(createImageView(image, _surfaceFormat.format, 1));
    }
}

void VulkanRenderer::deleteImageViews()
{
    for (auto imageView : _swapchainImageViews)
    {
        vkDestroyImageView(_device, imageView, nullptr);
    }
}

void VulkanRenderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0; // binding in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // used for image sampling

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 2;
    descriptorSetLayoutCreateInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan Descriptor Set layout");
    }

}

void VulkanRenderer::deleteDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
}

void VulkanRenderer::createGraphicsPipeline()
{
    // Shaders setup
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Triangle data setup
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
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
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan pipeline layout");
    }

    // Finally create pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineCreateInfo.pColorBlendState = &blendingCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.layout = _pipelineLayout;
    pipelineCreateInfo.renderPass = _renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no deriving from other pipeline
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_graphicsPipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan Graphics Pipeline");
    }

    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}

void VulkanRenderer::deleteGraphicsPipeline()
{
    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanRenderer::createRenderPass()
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

    // color attachment for MSAA
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
        throw std::runtime_error("Can't create Vulkan render pass");
    }
}

void VulkanRenderer::deleteRenderPass()
{
    vkDestroyRenderPass(_device, _renderPass, nullptr);
}

void VulkanRenderer::createFramebuffers()
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
            throw std::runtime_error("Can't create Vulkan Framebuffer");
        }
    }
}

void VulkanRenderer::deleteFramebuffers()
{
    for (auto framebuffer : _swapchainFramebuffers)
    {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
}

void VulkanRenderer::createCommandPool()
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = _queueIndex;

    if (vkCreateCommandPool(_device, &poolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan Command Pool");
    }
}

void VulkanRenderer::deleteCommandPool()
{
    vkDestroyCommandPool(_device, _commandPool, nullptr);
}

void VulkanRenderer::createMSAABuffer()
{
    VkFormat colorFormat = _surfaceFormat.format;

    createImage(_extent.width,
                _extent.height,
                1,
                _msaaSamples,
                colorFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _colorImage,
                _colorImageMemory);
    _colorImageView = createImageView(_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    transitionImageLayout(
            _colorImage,
            colorFormat,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            1);
}

void VulkanRenderer::deleteMSAABuffer()
{
    vkDestroyImageView(_device, _colorImageView, nullptr);
    vkDestroyImage(_device, _colorImage, nullptr);
    vkFreeMemory(_device, _colorImageMemory, nullptr);
}

void VulkanRenderer::createDepthBuffer()
{
    VkFormat depthFormat = findDepthFormat();
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    createImage(_extent.width,
                _extent.height,
                1,
                _msaaSamples,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _depthImage,
                _depthImageMemory);
    _depthImageView = createImageView(_depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(_depthImage,
                          depthFormat,
                          {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                          {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                           VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
                          1,
                          aspectFlags);
}

void VulkanRenderer::deleteDepthBuffer()
{
    vkDestroyImageView(_device, _depthImageView, nullptr);
    vkDestroyImage(_device, _depthImage, nullptr);
    vkFreeMemory(_device, _depthImageMemory, nullptr);
}

void VulkanRenderer::createTextureImage()
{
    // Load image pixel data
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/trashman_tex.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    _mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
        throw std::runtime_error("Can't load texture");
    }

    // Create temporary buffer to hold pixel data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    // Copy pixel data into temporary buffer
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(_device, stagingBufferMemory);

    // Free pixel data
    stbi_image_free(pixels);

    // Create texture image which will be used in shaders
    createImage(texWidth,
                texHeight,
                _mipLevels,
                VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _textureImage,
                _textureImageMemory);

    // Transition layout of the image to be optimal as a transfer destination
    transitionImageLayout(_textureImage,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                          {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
                          _mipLevels);
    // Copy image data from buffer to image
    copyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    // Transition image layout from transfer destination to optimal for shader usage
    /*transitionImageLayout(_textureImage,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
                          {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
                          _mipLevels);*/
    generateMipmaps(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, _mipLevels);

    // Free temporary buffer
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::deleteTextureImage()
{
    vkDestroyImage(_device, _textureImage, nullptr);
    vkFreeMemory(_device, _textureImageMemory, nullptr);
}

void VulkanRenderer::createTextureImageView()
{
    _textureImageView = createImageView(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, _mipLevels);
}

void VulkanRenderer::deleteTextureImageView()
{
    vkDestroyImageView(_device, _textureImageView, nullptr);
}

void VulkanRenderer::createTextureSampler()
{
    VkSamplerCreateInfo samplerCreateInfo {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.minLod = 0;
    samplerCreateInfo.maxLod = _mipLevels;
    samplerCreateInfo.mipLodBias = 0;

    if (vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan texture sampler");
    }
}

void VulkanRenderer::deleteTextureSampler()
{
    vkDestroySampler(_device, _textureSampler, nullptr);
}

void VulkanRenderer::loadModel()
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
            "models/trashman2.fbx",
            aiProcess_CalcTangentSpace       |
            aiProcess_Triangulate            |
            aiProcess_JoinIdenticalVertices  |
            aiProcess_SortByPType);

    // If the import failed, report it
    if(!scene)
    {
        throw std::runtime_error( importer.GetErrorString());
    }

    for (auto i = 0u; i < scene->mNumMeshes; i++)
    {
        const auto* mesh = scene->mMeshes[i];
        _vertices.reserve(_vertices.size() + mesh->mNumVertices);
        for (auto v = 0u; v < mesh->mNumVertices; v++)
        {
            Vertex vertex;
            vertex.pos = {mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z};
            vertex.color = {1.0f, 1.0f, 1.0f};
            vertex.texCoord = {mesh->mTextureCoords[0][v].x, 1.0f - mesh->mTextureCoords[0][v].y};
            _vertices.push_back(vertex);
        }
        _indices.reserve(_indices.size() + mesh->mNumFaces * 3);
        for (auto f = 0u; f < mesh->mNumFaces; f++)
        {
            for (auto index = 0u; index < mesh->mFaces[f].mNumIndices; index++)
            {
                _indices.push_back(mesh->mFaces[f].mIndices[index]);
            }
        }
    }

    // model freed in importer destructor
}

void VulkanRenderer::unloadModel()
{

}

void VulkanRenderer::createGeometryBuffers()
{
    // TODO: This can be optimized to use single buffer
    createOptimizedBuffer(_vertices, _vertexBuffer, _vertexBufferMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    createOptimizedBuffer(_indices, _indexBuffer, _indexBufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void VulkanRenderer::deleteGeometryBuffers()
{
    vkDestroyBuffer(_device, _indexBuffer, nullptr);
    vkFreeMemory(_device, _indexBufferMemory, nullptr);

    vkDestroyBuffer(_device, _vertexBuffer, nullptr);
    vkFreeMemory(_device, _vertexBufferMemory, nullptr);
}

void VulkanRenderer::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(MatricesUBO);

    _uniformBuffers.resize(_swapchainImages.size());
    _uniformBuffersMemory.resize(_swapchainImages.size());

    for (auto i = 0u; i < _uniformBuffers.size(); i++)
    {
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     _uniformBuffers[i],
                     _uniformBuffersMemory[i]);
    }
}

void VulkanRenderer::deleteUniformBuffers()
{
    for (auto i = 0u; i < _uniformBuffers.size(); i++)
    {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }
}

void VulkanRenderer::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = _uniformBuffers.size();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = _swapchainImages.size();

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = _swapchainImages.size();

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan descriptor pool");
    }
}

void VulkanRenderer::deleteDescriptorPool()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanRenderer::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(_uniformBuffers.size(), _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = _uniformBuffers.size();
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(_uniformBuffers.size());
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't allocate Vulkan descriptor sets");
    }

    for (auto i = 0u; i < _uniformBuffers.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO);

        VkDescriptorImageInfo imageInfo {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = _textureImageView;
        imageInfo.sampler = _textureSampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanRenderer::deleteDescriptorSets()
{

}

void VulkanRenderer::createCommandBuffers()
{
    _commandBuffers.resize(_swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = _commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) _commandBuffers.size();

    if (vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan Command Buffers");
    }

    for (size_t i = 0; i < _commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        // start recording
        if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording Vulkan command buffer");
        }

        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = _renderPass;
        renderPassBeginInfo.framebuffer = _swapchainFramebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = _extent;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

        VkDeviceSize offsets[] = {0};
        VkBuffer vertexBuffers[] = {_vertexBuffer};
        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(_commandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);

        vkCmdDrawIndexed(_commandBuffers[i], _indices.size(), 1, 0, 0, 0);
        vkCmdEndRenderPass(_commandBuffers[i]);

        // finish recording
        if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record Vulkan command buffer");
        }
    }
}

void VulkanRenderer::deleteCommandBuffers()
{

}

void VulkanRenderer::createSyncPrimitives()
{
    _imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphore[i]) != VK_SUCCESS
            || vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphore[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Vulkan semaphore");
        }

        if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Vulkan fence");
        }
    }
}

void VulkanRenderer::deleteSyncPrimitives()
{
    for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(_device, _renderFinishedSemaphore[i], nullptr);
        vkDestroySemaphore(_device, _imageAvailableSemaphore[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }
}

void VulkanRenderer::drawFrame()
{
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    // aquire image
    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX,
                                        _imageAvailableSemaphore[_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resizeWindow();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Can't aquire Vulkan Swapchain image");
    }

    updateUniformBuffer(imageIndex);

    // submit buffer to queue
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore[_currentFrame]};
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore[_currentFrame]};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages; // should be same size as wait semaphores
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[imageIndex]; // command buffer to submit (should be the one with current swap chain image attachment)
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);
    if (vkQueueSubmit(_queue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't submit Vulkan command buffer to queue");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(_queue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        resizeWindow();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Can't present swapchain image");
    }

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    MatricesUBO matrices {};
    matrices.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matrices.model = glm::rotate(matrices.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    matrices.view = glm::lookAt(glm::vec3(200.0f, 200.0f, 200.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matrices.projection = glm::perspective(glm::radians(45.0f), _extent.width / (float) _extent.height, 0.1f, 1000.0f);
    matrices.projection[1][1] *= -1; // because OpenGL use inverted Y axis

    void* data;
    vkMapMemory(_device, _uniformBuffersMemory[currentImage], 0, sizeof(matrices), 0, &data);
    memcpy(data, &matrices, sizeof(matrices));
    vkUnmapMemory(_device, _uniformBuffersMemory[currentImage]);
}

void VulkanRenderer::finishRendering()
{
    vkDeviceWaitIdle(_device);
}

void VulkanRenderer::resizeWindow()
{
    finishRendering();

    vkFreeCommandBuffers(_device, _commandPool, (uint32_t) _commandBuffers.size(), _commandBuffers.data());
    deleteMSAABuffer();
    deleteDepthBuffer();
    deleteFramebuffers();
    deleteGraphicsPipeline();
    deleteRenderPass();
    deleteImageViews();
    deleteSwapchain();
    deleteSurfaceParameters();

    createSurfaceParameters();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createMSAABuffer();
    createDepthBuffer();
    createFramebuffers();
    createCommandBuffers();
}

uint32_t VulkanRenderer::findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(_gpu, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Can't find suitable Vulkan device memory");
}

void VulkanRenderer::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory)
{
    // Create buffer
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan buffer");
    }

    // Allocate memory for buffer
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findVulkanMemoryType(memoryRequirements.memoryTypeBits, properties);

    // TODO: It's not very convenient to allocate new memory for each new buffer, as those allocations are limited
    // TODO: It can be refactored to use single allocation for many buffers (by using VulkanMemoryAllocator lib)
    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't allocate Vulkan buffer memory");
    }

    // Bind memory to buffer
    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

VkCommandBuffer VulkanRenderer::beginRecordingCommands()
{
    // Allocate command buffer for copy command
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    // Start recording commands
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanRenderer::endRecordingAndSubmitCommands(VkCommandBuffer commandBuffer)
{
    // Finish recording
    vkEndCommandBuffer(commandBuffer);

    // Submit command buffer to the queue for execution
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);

    // Wait until queue finish execution all commands
    vkQueueWaitIdle(_queue);

    // Free command buffer
    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    // copying buffer is similar to graphical commands -
    // copy commands should be added to command buffer and submitted to queue

    auto commandBuffer = beginRecordingCommands();

    VkBufferCopy copyRegion {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // copy buffer command

    endRecordingAndSubmitCommands(commandBuffer);
}

void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    auto commandBuffer = beginRecordingCommands();

    VkBufferImageCopy region {};
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
            1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endRecordingAndSubmitCommands(commandBuffer);
}

void VulkanRenderer::createImage(uint32_t width,
                                 uint32_t height,
                                 uint32_t mipLevels,
                                 VkSampleCountFlagBits numSamples,
                                 VkFormat format,
                                 VkImageTiling tiling,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkImage& image,
                                 VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageCreateInfo {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = numSamples; // used only for attachments
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(_device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan image");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(_device, image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findVulkanMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't allocate Vulkan image memory");
    }

    vkBindImageMemory(_device, image, imageMemory, 0);
}

VkImageView VulkanRenderer::createImageView(VkImage image,
                                            VkFormat format,
                                            uint32_t mipLevels,
                                            VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan Image view");
    }

    return imageView;
}


void VulkanRenderer::transitionImageLayout(VkImage image,
                                           VkFormat format,
                                           ImageLayoutState oldLayoutState,
                                           ImageLayoutState newLayoutState,
                                           uint32_t mipLevels,
                                           VkImageAspectFlags aspectFlags)
{
    // Layout transition can be achieved using memory barriers (which used for pipeline syncronization).
    // Memory barriers are pipeline command, which should be submitted to queue.

    VkCommandBuffer commandBuffer = beginRecordingCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayoutState.imageLayout;
    barrier.newLayout = newLayoutState.imageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = oldLayoutState.accessFlags;
    barrier.dstAccessMask = newLayoutState.accessFlags;

    vkCmdPipelineBarrier(
            commandBuffer,
            oldLayoutState.pipelineStageFlags,
            newLayoutState.pipelineStageFlags,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

    endRecordingAndSubmitCommands(commandBuffer);
}

// This method will create fast GPU-local buffer (using transitional temporary CPU visible buffer)
template <typename T>
void VulkanRenderer::createOptimizedBuffer(
        const std::vector<T>& bufferData,
        VkBuffer &buffer,
        VkDeviceMemory &deviceMemory,
        VkBufferUsageFlags usage)
{
    VkDeviceSize bufferSize = sizeof(bufferData[0]) * bufferData.size();

    // create temporary slow CPU visible memory
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);

    // Fill temporary buffer with vertex/index data
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bufferData.data(), (size_t) bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    // create fast GPU-local buffer for data
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 buffer,
                 deviceMemory);

    // copy slow CPU-visible buffer to fast GPU-local
    copyBuffer(stagingBuffer, buffer, bufferSize);

    // destroy CPU-visible buffer
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
}


VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                             VkImageTiling tiling,
                                             VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_gpu, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("Can't find suitable Vulkan format");
}

VkFormat VulkanRenderer::findDepthFormat()
{
    return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}


void VulkanRenderer::generateMipmaps(
        VkImage image,
        VkFormat imageFormat,
        int32_t texWidth,
        int32_t texHeight,
        uint32_t mipLevels)
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(_gpu, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("GPU does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = beginRecordingCommands();

    // Create template struct for transition commands
    VkImageMemoryBarrier barrier {};
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

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        // transfer prev image (or original) from transfer destination to transfer source
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit {};
        blit.srcOffsets[0] = {0, 0, 0}; // source region to blit
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};  // destination region to blit
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        // Blit previous mip image (or source image) to smaller mip level image
        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Transfer prev image (or original) to optimal for shader layout
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    endRecordingAndSubmitCommands(commandBuffer);
}

VkSampleCountFlagBits VulkanRenderer::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_gpu, &physicalDeviceProperties);

    VkSampleCountFlags counts = std::min(
            physicalDeviceProperties.limits.framebufferColorSampleCounts,
            physicalDeviceProperties.limits.framebufferDepthSampleCounts);
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}
