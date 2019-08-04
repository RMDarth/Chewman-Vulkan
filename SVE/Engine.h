// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include <SDL2/SDL.h>
#include "EngineSettings.h"
#include "SceneNode.h"

namespace SVE
{
class VulkanInstance;
class MaterialManager;
class SceneManager;
class ShaderManager;
class MeshManager;
class ResourceManager;
class ParticleSystemManager;

enum class CommandsType : uint8_t
{
    MainPass = 0,
    ShadowPassDirectLight,
    ShadowPassPointLights,
    // ShadowPassSpotLight,
    ReflectionPass,
    RefractionPass,
    ScreenQuadPass,
    ComputeParticlesPass
};

static const uint8_t PassCount = 6;

class Engine
{
public:
    static Engine* createInstance(SDL_Window* window, const std::string& settingsPath);
    static Engine* createInstance(SDL_Window* window, EngineSettings settings);
    static void destroyInstance(); // debug only
    static Engine* getInstance();
    ~Engine();

    VulkanInstance* getVulkanInstance();
    MaterialManager* getMaterialManager();
    MeshManager* getMeshManager();
    ShaderManager* getShaderManager();
    SceneManager* getSceneManager();
    ResourceManager* getResourceManager();
    ParticleSystemManager* getParticleSystemManager();

    void resizeWindow();
    void finishRendering();

    bool isShadowMappingEnabled() const;
    bool isWaterEnabled() const;

    CommandsType getPassType() const;
    float getTime();
    float getDeltaTime();

    void renderFrame();

private:
    Engine(SDL_Window* window, EngineSettings settings);
    explicit Engine(SDL_Window* window);

    void updateTime();
private:
    static Engine* _engineInstance;
    std::unique_ptr<VulkanInstance> _vulkanInstance;
    CommandsType _commandsType = CommandsType::MainPass;
    std::unique_ptr<MaterialManager> _materialManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::unique_ptr<MeshManager> _meshManager;
    std::unique_ptr<ShaderManager> _shaderManager;
    std::unique_ptr<ResourceManager> _resourceManager;
    std::unique_ptr<ParticleSystemManager> _particleSystemManager;

    std::chrono::high_resolution_clock::time_point _startTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point _currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point _prevTime = std::chrono::high_resolution_clock::now();
    float _duration;
    float _deltaTime;
};

} // namespace SVE