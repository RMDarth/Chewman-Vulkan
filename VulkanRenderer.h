#pragma once

#include "Renderer.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <SDL2/SDL.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class VulkanRenderer : public Renderer
{
public:
    explicit VulkanRenderer(SDL_Window *window, bool enableValidation = false);
    ~VulkanRenderer() override;

    void drawFrame() override;
    void finishRendering() override;
    void resizeWindow() override;
private:

    // Vulkan objects creators and destroyers
    void createInstance();
    void deleteInstance();
    void createDevice();
    void deleteDevice();
    void createSurface();
    void deleteSurface();
    void createSurfaceParameters();
    void deleteSurfaceParameters();
    void createSwapchain();
    void deleteSwapchain();
    void createImageViews();
    void deleteImageViews();
    void createRenderPass();
    void deleteRenderPass();
    void createDescriptorSetLayout();
    void deleteDescriptorSetLayout();
    void createGraphicsPipeline();
    void deleteGraphicsPipeline();
    void createCommandPool();
    void deleteCommandPool();
    void createMSAABuffer();
    void deleteMSAABuffer();
    void createDepthBuffer();
    void deleteDepthBuffer();
    void createFramebuffers();
    void deleteFramebuffers();
    void createTextureImage();
    void deleteTextureImage();
    void createTextureImageView();
    void deleteTextureImageView();
    void createTextureSampler();
    void deleteTextureSampler();
    void loadModel();
    void unloadModel();
    void createGeometryBuffers();
    void deleteGeometryBuffers();
    void createUniformBuffers();
    void deleteUniformBuffers();
    void createDescriptorPool();
    void deleteDescriptorPool();
    void createDescriptorSets();
    void deleteDescriptorSets();
    void createCommandBuffers();
    void deleteCommandBuffers();
    void createSyncPrimitives();
    void deleteSyncPrimitives();

    void createDebugCallback();
    void deleteDebugCallback();

    /// Helper functions
    struct ImageLayoutState
    {
        VkImageLayout imageLayout;
        VkAccessFlags accessFlags;
        VkPipelineStageFlags pipelineStageFlags;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

    VkCommandBuffer beginRecordingCommands();
    void endRecordingAndSubmitCommands(VkCommandBuffer commandBuffer);

    void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImage(uint32_t width,
                     uint32_t height,
                     uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image,
                     VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image,
                                VkFormat format,
                                uint32_t mipLevels,
                                VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    void transitionImageLayout(VkImage image,
                               VkFormat format,
                               ImageLayoutState oldLayoutState,
                               ImageLayoutState newLayoutState,
                               uint32_t mipLevels,
                               VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

    template <typename T>
    void createOptimizedBuffer(
            const std::vector<T>& data,
            VkBuffer &buffer,
            VkDeviceMemory &deviceMemory,
            VkBufferUsageFlags usage);

    void addPlatformSpecificExtensions(std::vector<const char*>& extensionsList);

    void updateUniformBuffer(uint32_t currentImage);

private:
    bool _enableValidation;

    SDL_Window *_window = nullptr;
    int _windowWidth;
    int _windowHeight;

    VkInstance _instance = VK_NULL_HANDLE;
    VkPhysicalDevice _gpu = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties _gpuProps = {};
    VkDevice _device = VK_NULL_HANDLE;

    VkDebugReportCallbackEXT _debugCallbackHandle = VK_NULL_HANDLE;

    VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    uint32_t _queueIndex;
    VkQueue _queue = VK_NULL_HANDLE;

    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSurfaceFormatKHR _surfaceFormat = {};
    VkSurfaceCapabilitiesKHR _surfaceCapabilities = {};

    VkPresentModeKHR _presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D _extent;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkRenderPass _renderPass = VK_NULL_HANDLE;

    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout  _pipelineLayout = VK_NULL_HANDLE;
    VkPipeline _graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    std::vector<VkFramebuffer> _swapchainFramebuffers;

    // color attachment for anti-aliasing
    VkImage _colorImage;
    VkDeviceMemory _colorImageMemory;
    VkImageView _colorImageView;

    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;

    uint32_t _mipLevels;
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;

    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    VkBuffer _vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer _indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;

    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> _descriptorSets;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> _imageAvailableSemaphore;
    std::vector<VkSemaphore> _renderFinishedSemaphore;
    std::vector<VkFence> _inFlightFences;
    size_t _currentFrame = 0;
};
