// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include "Engine.h"
#include "VulkanUtils.h"
#include "VulkanHeaders.h"
#include <vector>
#include <SDL2/SDL.h>
#include <map>

namespace SVE
{
class VulkanMesh;
class VulkanScreenQuad;
class VulkanSamplerHolder;
class VulkanPassInfo;

// TODO: Create some mapping to external indexes instead of hardcoding
enum
{
    BUFFER_INDEX_SHADOWMAP_SUN = 100,
    BUFFER_INDEX_SHADOWMAP_POINT = 200,
    BUFFER_INDEX_WATER_REFLECTION = 300,
    BUFFER_INDEX_WATER_REFRACTION = 301,
    BUFFER_INDEX_SCREEN_QUAD_DEPTH = 350,
    BUFFER_INDEX_SCREEN_QUAD = 400,
    BUFFER_INDEX_SCREEN_QUAD_MRT = 450,
    BUFFER_INDEX_SCREEN_QUAD_LATE = 451,
    BUFFER_INDEX_COMPUTE_PARTICLES = 500
};

using PoolID = uint32_t;
using BufferIndex = uint32_t;

constexpr PoolID ServicePoolIndex = 100;

class VulkanInstance
{
public:
    VulkanInstance(SDL_Window* window, EngineSettings settings);
    ~VulkanInstance();

    const VulkanUtils& getVulkanUtils() const;
    const EngineSettings& getEngineSettings() const;

    void resizeWindow();
    void finishRendering() const;

    void onPause();
    void onResume();

    VkInstance getInstance() const;
    VkPhysicalDevice getGPU() const;
    VkDevice getLogicalDevice() const;
    VkCommandPool getCommandPool(PoolID index) const;
    VkRenderPass getRenderPass() const;
    VkExtent2D getExtent() const;
    SDL_Window* getWindow() const;
    VkSampleCountFlagBits getMSAASamples() const;
    VkQueue getGraphicsQueue() const;
    size_t getSwapchainSize() const;
    size_t getInFlightSize() const;
    VkFormat getSurfaceColorFormat() const;
    VkFormat getDepthFormat() const;
    VkImageAspectFlags getDepthAspectFlags(VkFormat depthFormat) const;
    VkFramebuffer getFramebuffer(size_t index) const;

    VkCommandBuffer createCommandBuffer(BufferIndex bufferIndex);
    VkCommandBuffer getCommandBuffer(BufferIndex index) const;

    const std::vector<VkCommandBuffer>& getCommandBuffersList();

    void waitAvailableFramebuffer();
    void submitCommands(CommandsType commandsType, BufferIndex bufferIndex) const;
    void renderCommands() const;
    uint32_t getCurrentImageIndex() const;
    uint32_t getCurrentFrameIndex() const;

    void reallocateCommandBuffers();
    void startRenderCommandBufferCreation();
    void endRenderCommandBufferCreation();

    VulkanScreenQuad* getScreenQuad();
    VulkanSamplerHolder* getSamplerHolder();
    VulkanPassInfo* getPassInfo();
    void initScreenQuad(glm::ivec2 resolution);

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
    void createCommandPool();
    void deleteCommandPool();
    void createMSAABuffer();
    void deleteMSAABuffer();
    void createDepthBuffer();
    void deleteDepthBuffer();
    void createFramebuffers();
    void deleteFramebuffers();
    void createSyncPrimitives();
    void deleteSyncPrimitives();

    void createDebugCallback();
    void deleteDebugCallback();

private:
    // Utility functions
    void addPlatformSpecificExtensions(std::vector<const char*>& extensionsList);
    VkSampleCountFlagBits getMSAALevelsValue(int msaaLevels);
    size_t getGPUIndex(std::vector<VkPhysicalDevice>& deviceList);

private:
    EngineSettings _engineSettings;
    VulkanUtils _vulkanUtils;

    SDL_Window *_window;
    int _windowWidth;
    int _windowHeight;

    VkInstance _instance;
    VkPhysicalDevice _gpu;
    VkPhysicalDeviceProperties _gpuProps;
    VkDevice _device;

    VkDebugReportCallbackEXT _debugCallbackHandle;
    VkDebugUtilsMessengerEXT _debugUtilsCallbackHandle;

    VkSampleCountFlagBits _msaaSamples;

    uint32_t _queueIndex;
    VkQueue _queue;

    VkSurfaceKHR _surface;
    VkSurfaceFormatKHR _surfaceFormat;
    VkSurfaceCapabilitiesKHR _surfaceCapabilities;

    VkPresentModeKHR _presentMode;
    VkExtent2D _extent;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkRenderPass _renderPass;

    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    std::vector<VkFramebuffer> _swapchainFramebuffers;

    PoolID _currentPool;
    std::vector<VkCommandPool> _commandPools;
    VkCommandPool _servicePool; // pool for service allocations
    std::vector<VkCommandBuffer> _commandBuffers;
    std::map<uint32_t, VkCommandBuffer> _externalBufferMap;
    std::map<std::pair<PoolID, BufferIndex>, VkCommandBuffer> _poolBufferMap;

    // color attachment for anti-aliasing
    VkImage _colorImage;
    VkDeviceMemory _colorImageMemory;
    VkImageView _colorImageView;

    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 3; // max parallel processing frame
    std::vector<VkSemaphore> _computeParticlesReadySemaphore;
    std::vector<VkSemaphore> _shadowMapDirectReadySemaphores;
    std::vector<VkSemaphore> _shadowMapPointReadySemaphores;
    std::vector<VkSemaphore> _waterReflectionReadySemaphores;
    std::vector<VkSemaphore> _waterRefractionReadySemaphores;
    std::vector<VkSemaphore> _screenQuadReadySemaphores;
    std::vector<VkSemaphore> _screenQuadMrtReadySemaphores;
    std::vector<VkSemaphore> _screenQuadLateReadySemaphores;
    std::vector<VkSemaphore> _screenQuadDepthReadySemaphores;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;

    // TODO: Meh... mutable
    mutable int _currentFrame = 0;
    mutable VkSemaphore _currentWaitSemaphore;

    std::vector<VkFence> _inFlightFences;
    uint32_t _currentImageIndex = 0;

    std::unique_ptr<VulkanScreenQuad> _screenQuad;
    std::unique_ptr<VulkanSamplerHolder> _samplerHolder;
    std::unique_ptr<VulkanPassInfo> _passInfo;
};

} // namespace SVE