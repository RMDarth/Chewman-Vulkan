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
class SceneNodel;

class Engine
{
public:
    static Engine* createInstance(SDL_Window* window, EngineSettings settings);
    static Engine* getInstance();
    ~Engine();

    VulkanInstance* getVulkanInstance();
    MaterialManager* getMaterialManager();
    ShaderManager* getShaderManager();
    SceneManager* getSceneManager();

    void renderFrame();

private:
    Engine(SDL_Window* window, EngineSettings settings);
    explicit Engine(SDL_Window* window);
private:
    static Engine* _engineInstance;
    VulkanInstance* _vulkanInstance;
    std::unique_ptr<MaterialManager> _materialManager;
    std::unique_ptr<SceneManager> _sceneManager;
    std::unique_ptr<ShaderManager> _shaderManager;
};

} // namespace SVE