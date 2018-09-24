#include <utility>

// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanShadowMap.h"
#include "VulkanException.h"
#include "MaterialManager.h"
#include "SceneManager.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "LightNode.h"
#include "Skybox.h"
#include "ShadowMap.h"
#include <chrono>

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
        _engineInstance = new Engine(window, std::move(settings));
    }
    return _engineInstance;
}

Engine* Engine::createInstance(SDL_Window* window, const std::string& settingsPath)
{
    if (_engineInstance == nullptr)
    {
        auto data = ResourceManager::getLoadDataFromFolder(settingsPath);
        if (data.engine.empty())
        {
            throw VulkanException("Can't find SVE configuration file");
        }
        _engineInstance = new Engine(window, data.engine.front());

        // TODO: Revise shadowmap creation sequence
        _engineInstance->getSceneManager()->initShadowMap();
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
    , _resourceManager(std::make_unique<ResourceManager>())
{
    getTime();
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

ResourceManager* Engine::getResourceManager()
{
    return _resourceManager.get();
}

MeshManager* Engine::getMeshManager()
{
    return _meshManager.get();
}


void Engine::resizeWindow()
{
    _sceneManager->queueCommandBuffersUpdate();
    _vulkanInstance->resizeWindow();
    _materialManager->resetPipelines();

    _sceneManager->getMainCamera()->setAspectRatio(
            (float)_vulkanInstance->getExtent().width / _vulkanInstance->getExtent().height);
}

void Engine::finishRendering()
{
    _vulkanInstance->finishRendering();
}

void createNodeDrawCommands(std::shared_ptr<SceneNode> node, uint32_t bufferIndex)
{
    for (auto& entity : node->getAttachedEntities())
    {
        entity->applyDrawingCommands(bufferIndex, bufferIndex != SHADOWMAP_BUFFER_INDEX);
    }

    for (auto& child : node->getChildren())
    {
        createNodeDrawCommands(child, bufferIndex);
    }
}

void updateNode(const std::shared_ptr<SceneNode>& node, UniformData& uniformData, UniformData& uniformShadowData)
{
    auto oldModel = uniformData.model;
    uniformData.model *= node->getNodeTransformation();
    uniformShadowData.model = uniformData.model;

    // update uniforms
    for (auto& entity : node->getAttachedEntities())
    {
        entity->updateUniforms(uniformData, uniformShadowData);
    }

    for (auto& child : node->getChildren())
    {
        updateNode(child, uniformData, uniformShadowData);
    }

    uniformData.model = oldModel;
    uniformShadowData.model = oldModel;
}

void Engine::renderFrame()
{
    auto skybox = _sceneManager->getSkybox();

    // update command buffers if scene changed
    if (_sceneManager->isCommandBufferUpdateQueued())
    {
        _vulkanInstance->reallocateCommandBuffers();

        auto shadowMap = _sceneManager->getShadowMap();
        if (shadowMap->isEnabled())
        {
            shadowMap->getVulkanShadowMap()->reallocateCommandBuffers();
            shadowMap->getVulkanShadowMap()->startRenderCommandBufferCreation();
            createNodeDrawCommands(_sceneManager->getRootNode(), SHADOWMAP_BUFFER_INDEX);
            shadowMap->getVulkanShadowMap()->endRenderCommandBufferCreation();
        }

        for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); ++i)
        {
            _vulkanInstance->startRenderCommandBufferCreation(i);
            if (skybox)
                skybox->applyDrawingCommands(i, true);
            createNodeDrawCommands(_sceneManager->getRootNode(), i);
            _vulkanInstance->endRenderCommandBufferCreation(i);
        }
        _sceneManager->dequeueCommandBufferUpdate();
    }

    // Fill uniform data (from camera and lights)
    auto mainCamera = _sceneManager->getMainCamera();
    if (!mainCamera)
        throw VulkanException("Camera not set");
    UniformData uniformData;
    _sceneManager->getMainCamera()->fillUniformData(uniformData);
    UniformData shadowUniformData = uniformData;
    if (_sceneManager->getLight())
    {
        _sceneManager->getLight()->fillUniformData(uniformData, false);
        _sceneManager->getLight()->fillUniformData(shadowUniformData, true);
    }
    _vulkanInstance->waitAvailableFramebuffer();

    // update uniforms
    if (skybox)
        skybox->updateUniforms(uniformData, shadowUniformData);


    // submit command buffers
    updateNode(_sceneManager->getRootNode(), uniformData, shadowUniformData);
    _vulkanInstance->submitCommands(CommandsType::ShadowPass);
    _vulkanInstance->submitCommands(CommandsType::MainPass);
    _vulkanInstance->renderCommands();
}

float Engine::getTime()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
}

bool Engine::isShadowMappingEnabled() const
{
    return _sceneManager->getShadowMap()->isEnabled();
}

} // namespace SVE