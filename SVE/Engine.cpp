// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Engine.h"
#include "VulkanInstance.h"
#include "MaterialManager.h"
#include "SceneManager.h"
#include "ShaderManager.h"
#include "Entity.h"

namespace SVE
{

Engine* Engine::_engineInstance = nullptr;

Engine* Engine::getInstance()
{
    return _engineInstance;
}

Engine* Engine::createInstance(SDL_Window* window, EngineSettings settings)
{
    if (_engineInstance == nullptr)
    {
        _engineInstance = new Engine(window, settings);
    }
    return _engineInstance;
}

VulkanInstance* Engine::getVulkanInstance()
{
    return _vulkanInstance;
}

Engine::Engine(SDL_Window* window)
    : Engine(window, EngineSettings())
{
}

Engine::Engine(SDL_Window* window, EngineSettings settings)
    : _vulkanInstance(new VulkanInstance(window, std::move(settings)))
    , _materialManager(new MaterialManager())
    , _shaderManager(std::make_unique<ShaderManager>())
    , _sceneManager(new SceneManager())
{

}

Engine::~Engine() = default;

MaterialManager* Engine::getMaterialManager()
{
    return _materialManager.get();
}

ShaderManager* Engine::getShaderManager()
{
    return _shaderManager.get();
}

SceneManager* Engine::getSceneManager()
{
    return _sceneManager.get();
}

void renderNode(std::shared_ptr<SceneNode> node, std::vector<SubmitInfo>& submitList)
{
    // render node
    for (auto& entity : node->getAttachedEntities())
    {
        submitList.emplace_back(std::move(entity->render()));
    }

    for (auto& child : node->getChildren())
    {
        renderNode(child, submitList);
    }
}

void Engine::renderFrame()
{
    _vulkanInstance->waitAvailableFramebuffer();
    std::vector<SubmitInfo> submitList;
    renderNode(_sceneManager->getRootNode(), submitList);
    _vulkanInstance->submitCommands(submitList);
}

} // namespace SVE