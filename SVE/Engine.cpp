// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanException.h"
#include "MaterialManager.h"
#include "SceneManager.h"
#include "ShaderManager.h"
#include "MeshManager.h"
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
    , _meshManager(std::make_unique<MeshManager>())
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

MeshManager* Engine::getMeshManager()
{
    return _meshManager.get();
}

void renderNode(std::shared_ptr<SceneNode> node, UniformData& uniformData, std::vector<SubmitInfo>& submitList)
{
    auto oldModel = uniformData.model;
    uniformData.model *= node->getNodeTransformation();
    // render node
    for (auto& entity : node->getAttachedEntities())
    {
        auto submitInfo = entity->render(uniformData);
        if (!submitInfo.isEmpty())
            submitList.emplace_back(std::move(submitInfo));
    }

    for (auto& child : node->getChildren())
    {
        renderNode(child, uniformData, submitList);
    }
    uniformData.model = oldModel;
}

void Engine::renderFrame()
{
    auto mainCamera = _sceneManager->getMainCamera();
    if (!mainCamera)
        throw VulkanException("Camera not set");
    UniformData uniformData = _sceneManager->getMainCamera()->fillUniformData();
    _vulkanInstance->waitAvailableFramebuffer();
    std::vector<SubmitInfo> submitList;
    renderNode(_sceneManager->getRootNode(), uniformData, submitList);
    _vulkanInstance->submitCommands(submitList);
}

} // namespace SVE