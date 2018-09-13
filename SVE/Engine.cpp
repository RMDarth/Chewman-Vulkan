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
#include "LightNode.h"

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

void createNodeDrawCommands(std::shared_ptr<SceneNode> node, uint32_t bufferIndex)
{
    for (auto& entity : node->getAttachedEntities())
    {
        entity->applyDrawingCommands(bufferIndex);
    }

    for (auto& child : node->getChildren())
    {
        createNodeDrawCommands(child, bufferIndex);
    }
}

void updateNode(const std::shared_ptr<SceneNode>& node, UniformData& uniformData)
{
    auto oldModel = uniformData.model;
    uniformData.model *= node->getNodeTransformation();

    // update uniforms
    for (auto& entity : node->getAttachedEntities())
    {
        entity->updateUniforms(uniformData);
    }

    for (auto& child : node->getChildren())
    {
        updateNode(child, uniformData);
    }

    uniformData.model = oldModel;
}

void Engine::renderFrame()
{
    if (_sceneManager->isCommandBufferUpdateQueued())
    {
        _vulkanInstance->reallocateCommandBuffers();
        for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); ++i)
        {
            _vulkanInstance->startRenderCommandBufferCreation(i);
            createNodeDrawCommands(_sceneManager->getRootNode(), i);
            _vulkanInstance->endRenderCommandBufferCreation(i);
        }
        _sceneManager->dequeueCommandBufferUpdate();
    }

    auto mainCamera = _sceneManager->getMainCamera();
    if (!mainCamera)
        throw VulkanException("Camera not set");
    UniformData uniformData;
    _sceneManager->getMainCamera()->fillUniformData(uniformData);
    if (_sceneManager->getLight())
        _sceneManager->getLight()->fillUniformData(uniformData);
    _vulkanInstance->waitAvailableFramebuffer();
    updateNode(_sceneManager->getRootNode(), uniformData);
    _vulkanInstance->submitCommands();
}

} // namespace SVE