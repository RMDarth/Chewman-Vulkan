#include <utility>

// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanShadowMap.h"
#include "VulkanWater.h"
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
#include "Water.h"
#include "Utils.h"
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

        auto settings = data.engine.front();
        _engineInstance = new Engine(window, settings);

        if (settings.initShadows)
            _engineInstance->getSceneManager()->initShadowMap();
        if (settings.initWater)
            _engineInstance->getSceneManager()->createWater(0);
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

void createNodeDrawCommands(const std::shared_ptr<SceneNode>& node, uint32_t bufferIndex)
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

void updateNode(const std::shared_ptr<SceneNode>& node, UniformDataList& uniformDataList)
{
    auto oldModel = uniformDataList[toInt(CommandsType::MainPass)]->model;
    uniformDataList[toInt(CommandsType::MainPass)]->model *= node->getNodeTransformation();
    for (auto i = 1u; i < PassCount; i++)
        uniformDataList[i]->model = uniformDataList[toInt(CommandsType::MainPass)]->model;

    // update uniforms
    for (auto& entity : node->getAttachedEntities())
    {
        entity->updateUniforms(uniformDataList);
    }

    for (auto& child : node->getChildren())
    {
        updateNode(child, uniformDataList);
    }

    for (auto i = 0u; i < PassCount; i++)
        uniformDataList[i]->model = oldModel;
}

void Engine::renderFrame()
{
    auto skybox = _sceneManager->getSkybox();

    // update command buffers if scene changed
    if (_sceneManager->isCommandBufferUpdateQueued())
    {
        _vulkanInstance->reallocateCommandBuffers();

        _commandsType = CommandsType::ShadowPass;
        auto shadowMap = _sceneManager->getShadowMap();
        if (shadowMap->isEnabled())
        {
            shadowMap->getVulkanShadowMap()->reallocateCommandBuffers();
            shadowMap->getVulkanShadowMap()->startRenderCommandBufferCreation();
            createNodeDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SHADOWMAP);
            shadowMap->getVulkanShadowMap()->endRenderCommandBufferCreation();
        }

        if (auto water = _sceneManager->getWater())
        {
            water->getVulkanWater()->reallocateCommandBuffers();
            _commandsType = CommandsType::ReflectionPass;
            water->getVulkanWater()->startRenderCommandBufferCreation(VulkanWater::PassType::Reflection);
            if (skybox)
                skybox->applyDrawingCommands(BUFFER_INDEX_WATER_REFLECTION);
            createNodeDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_WATER_REFLECTION);
            water->getVulkanWater()->endRenderCommandBufferCreation(VulkanWater::PassType::Reflection);

            _commandsType = CommandsType::RefractionPass;
            water->getVulkanWater()->startRenderCommandBufferCreation(VulkanWater::PassType::Refraction);
            if (skybox)
                skybox->applyDrawingCommands(BUFFER_INDEX_WATER_REFRACTION);
            createNodeDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_WATER_REFRACTION);
            water->getVulkanWater()->endRenderCommandBufferCreation(VulkanWater::PassType::Refraction);
        }

        _commandsType = CommandsType::MainPass;
        for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); ++i)
        {
            _vulkanInstance->startRenderCommandBufferCreation(i);
            if (skybox)
                skybox->applyDrawingCommands(i);
            createNodeDrawCommands(_sceneManager->getRootNode(), i);
            _vulkanInstance->endRenderCommandBufferCreation(i);
        }
        _sceneManager->dequeueCommandBufferUpdate();
    }

    // Fill uniform data (from camera and lights)
    auto mainCamera = _sceneManager->getMainCamera();
    if (!mainCamera)
        throw VulkanException("Camera not set");\
    UniformDataList uniformDataList(4);
    for (auto i = 0; i < PassCount; i++)
    {
        uniformDataList[i] = std::make_shared<UniformData>();
    }
    auto mainUniform = uniformDataList[toInt(CommandsType::MainPass)];

    mainUniform->clipPlane = glm::vec4(0.0, 1.0, 0.0, 500);
    mainUniform->time = getTime();
    _sceneManager->getMainCamera()->fillUniformData(*mainUniform);

    for (auto i = 1; i < PassCount; i++)
    {
        *uniformDataList[i] = *uniformDataList[0];
    }
    if (_sceneManager->getLight())
    {
        for (auto i = 0; i < PassCount; i++)
        {
            _sceneManager->getLight()->fillUniformData(*uniformDataList[i], i == toInt(CommandsType::ShadowPass));
        }
    }
    if (auto water = _sceneManager->getWater())
    {
        water->getVulkanWater()->fillUniformData(*uniformDataList[toInt(CommandsType::ReflectionPass)],
                                                 VulkanWater::PassType::Reflection);
        water->getVulkanWater()->fillUniformData(*uniformDataList[toInt(CommandsType::RefractionPass)],
                                                 VulkanWater::PassType::Refraction);
    }
    _vulkanInstance->waitAvailableFramebuffer();

    // update uniforms
    if (skybox)
        skybox->updateUniforms(uniformDataList);

    // submit command buffers
    updateNode(_sceneManager->getRootNode(), uniformDataList);
    _vulkanInstance->submitCommands(CommandsType::ShadowPass);
    if (auto water = _sceneManager->getWater())
    {
        _vulkanInstance->submitCommands(CommandsType::ReflectionPass);
        _vulkanInstance->submitCommands(CommandsType::RefractionPass);
    }

    _vulkanInstance->submitCommands(CommandsType::MainPass);
    _vulkanInstance->renderCommands();
}

float Engine::getTime()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
}

CommandsType Engine::getPassType() const
{
    return _commandsType;
}

bool Engine::isShadowMappingEnabled() const
{
    return _sceneManager->getShadowMap()->isEnabled();
}

bool Engine::isWaterEnabled() const
{
    return _sceneManager->getWater() != nullptr;
}

} // namespace SVE