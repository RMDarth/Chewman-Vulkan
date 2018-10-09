// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#pragma once
#include <vector>
#include <memory>
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

enum class CommandsType : uint8_t
{
    MainPass = 0,
    ShadowPass,
    ReflectionPass,
    RefractionPass,
    ScreenQuadPass,
};

static const uint8_t PassCount = 5;

class Engine
{
public:
    static Engine* createInstance(SDL_Window* window, const std::string& settingsPath);
    static Engine* createInstance(SDL_Window* window, EngineSettings settings);
    static Engine* getInstance();
    ~Engine();

    VulkanInstance* getVulkanInstance();
    MaterialManager* getMaterialManager();
    MeshManager* getMeshManager();
    ShaderManager* getShaderManager();
    SceneManager* getSceneManager();
    ResourceManager* getResourceManager();

    void resizeWindow();
    void finishRendering();

    bool isShadowMappingEnabled() const;
    bool isWaterEnabled() const;

    CommandsType getPassType() const;
    float getTime();

    void renderFrame();

private:
    Engine(SDL_Window* window, EngineSettings settings);
    explicit Engine(SDL_Window* window);
private:
    static Engine* _engineInstance;
    VulkanInstance* _vulkanInstance;
    CommandsType _commandsType = CommandsType::MainPass;
    std::unique_ptr<MaterialManager> _materialManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::unique_ptr<MeshManager> _meshManager;
    std::unique_ptr<ShaderManager> _shaderManager;
    std::unique_ptr<ResourceManager> _resourceManager;
};

} // namespace SVE