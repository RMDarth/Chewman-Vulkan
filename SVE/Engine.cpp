// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanShadowMap.h"
#include "VulkanWater.h"
#include "VulkanScreenQuad.h"
#include "VulkanException.h"
#include "VulkanMaterial.h"
#include "VulkanShadowImage.h"
#include "MaterialManager.h"
#include "SceneManager.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "LightManager.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "Skybox.h"
#include "ShadowMap.h"
#include "Water.h"
#include "Utils.h"
#include <chrono>
#include <utility>

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

        //if (settings.initShadows)
        _engineInstance->getSceneManager()->getLightManager();
        //    _engineInstance->getSceneManager()->initShadowMap();
        if (settings.initWater)
            _engineInstance->getSceneManager()->createWater(0);
        if (settings.useScreenQuad)
        {
            _engineInstance->getVulkanInstance()->initScreenQuad();
        }
    }
    return _engineInstance;
}

VulkanInstance* Engine::getVulkanInstance()
{
    return _vulkanInstance.get();
}

Engine::Engine(SDL_Window* window)
    : Engine(window, EngineSettings())
{
}

Engine::Engine(SDL_Window* window, EngineSettings settings)
    : _vulkanInstance(std::make_unique<VulkanInstance>(window, std::move(settings)))
    , _materialManager(std::make_unique<MaterialManager>())
    , _shaderManager(std::make_unique<ShaderManager>())
    , _sceneManager(std::make_unique<SceneManager>())
    , _meshManager(std::make_unique<MeshManager>())
    , _resourceManager(std::make_unique<ResourceManager>())
{
    getTime();
}

Engine::~Engine()
{
    // TODO: need to add correct resource handling
    _resourceManager.reset();
    _meshManager.reset();
    _sceneManager.reset();
    _shaderManager.reset();
    _materialManager.reset();
    _vulkanInstance.reset();
}

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
    _vulkanInstance->resizeWindow();
    _materialManager->resetPipelines();

    _sceneManager->getMainCamera()->setAspectRatio(
            (float)_vulkanInstance->getExtent().width / _vulkanInstance->getExtent().height);
}

void Engine::finishRendering()
{
    _vulkanInstance->finishRendering();
}

void createNodeDrawCommands(const std::shared_ptr<SceneNode>& node, uint32_t bufferIndex, uint32_t imageIndex)
{
    for (auto& entity : node->getAttachedEntities())
    {
        entity->applyDrawingCommands(bufferIndex, imageIndex);
    }

    for (auto& child : node->getChildren())
    {
        createNodeDrawCommands(child, bufferIndex, imageIndex);
    }
}

void updateNode(const std::shared_ptr<SceneNode>& node, UniformDataList& uniformDataList, const UniformDataIndexMap& indexMap)
{
    auto oldModel = uniformDataList[0]->model;
    uniformDataList[0]->model *= node->getNodeTransformation();
    for (auto i = 1u; i < uniformDataList.size(); i++)
        uniformDataList[i]->model = uniformDataList[0]->model;

    // update uniforms
    for (auto& entity : node->getAttachedEntities())
    {
        entity->updateUniforms(uniformDataList, indexMap);
    }

    for (auto& child : node->getChildren())
    {
        updateNode(child, uniformDataList, indexMap);
    }

    for (auto i = 0u; i < uniformDataList.size(); i++)
        uniformDataList[i]->model = oldModel;
}

void Engine::renderFrame()
{
    _vulkanInstance->waitAvailableFramebuffer();
    updateTime();
    auto skybox = _sceneManager->getSkybox();
    auto currentFrame = _vulkanInstance->getCurrentFrameIndex();
    auto currentImage = _vulkanInstance->getCurrentImageIndex();

    ////// update command buffers

    _vulkanInstance->reallocateCommandBuffers();

    _commandsType = CommandsType::ShadowPass;
    for (auto i = 0u; i < _sceneManager->getLightManager()->getLightCount(); i++)
    {
        _passSubIndex = i;
        auto shadowMap = _sceneManager->getLightManager()->getLight(i)->getShadowMap();

        if (shadowMap && shadowMap->isEnabled())
        {
            shadowMap->getVulkanShadowMap()->reallocateCommandBuffers();

            auto bufferIndex =
                    shadowMap->getVulkanShadowMap()->startRenderCommandBufferCreation(
                            _vulkanInstance->getCurrentFrameIndex(),
                            _vulkanInstance->getCurrentImageIndex());
            createNodeDrawCommands(_sceneManager->getRootNode(), bufferIndex, currentImage);
            shadowMap->getVulkanShadowMap()->endRenderCommandBufferCreation(
                    _vulkanInstance->getCurrentFrameIndex());
        }
    }

    if (auto water = _sceneManager->getWater())
    {
        water->getVulkanWater()->reallocateCommandBuffers();
        _commandsType = CommandsType::ReflectionPass;
        water->getVulkanWater()->startRenderCommandBufferCreation(VulkanWater::PassType::Reflection);
        if (skybox)
            skybox->applyDrawingCommands(BUFFER_INDEX_WATER_REFLECTION, currentImage);
        createNodeDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_WATER_REFLECTION, currentImage);
        water->getVulkanWater()->endRenderCommandBufferCreation(VulkanWater::PassType::Reflection);

        _commandsType = CommandsType::RefractionPass;
        water->getVulkanWater()->startRenderCommandBufferCreation(VulkanWater::PassType::Refraction);
        if (skybox)
            skybox->applyDrawingCommands(BUFFER_INDEX_WATER_REFRACTION, currentImage);
        createNodeDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_WATER_REFRACTION, currentImage);
        water->getVulkanWater()->endRenderCommandBufferCreation(VulkanWater::PassType::Refraction);
    }

    if (auto* screenQuad = _vulkanInstance->getScreenQuad())
    {
        _commandsType = CommandsType::ScreenQuadPass;
        screenQuad->reallocateCommandBuffers();
        screenQuad->startRenderCommandBufferCreation();
        if (skybox)
            skybox->applyDrawingCommands(BUFFER_INDEX_SCREEN_QUAD, currentImage);
        createNodeDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD, currentImage);
        screenQuad->endRenderCommandBufferCreation();
    }

    _commandsType = CommandsType::MainPass;
    _vulkanInstance->startRenderCommandBufferCreation();
    if (auto* screenQuad = _vulkanInstance->getScreenQuad())
    {
        _engineInstance->getMaterialManager()->getMaterial("ScreenQuad")->getVulkanMaterial()->getInstanceForEntity(nullptr);
        _materialManager->getMaterial("ScreenQuad")->getVulkanMaterial()->applyDrawingCommands(currentFrame, currentImage, 1);
        auto commandBuffer = _vulkanInstance->getCommandBuffer(currentFrame);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    } else
    {
        if (skybox)
            skybox->applyDrawingCommands(currentFrame, currentImage);
        createNodeDrawCommands(_sceneManager->getRootNode(), currentFrame, currentImage);
    }
    _vulkanInstance->endRenderCommandBufferCreation();


    ////// Fill uniform data (from camera and lights)

    auto mainCamera = _sceneManager->getMainCamera();
    if (!mainCamera)
        throw VulkanException("Camera not set");

    uint32_t PassCount = 0;
    UniformDataIndexMap indexMap;
    for (auto i = 0u; i < PassTypeCount; i++)
    {
        indexMap[static_cast<CommandsType>(i)] = PassCount;
        if (i == toInt(CommandsType::ShadowPass))
        {
            PassCount += _sceneManager->getLightManager()->getLightCount();
        } else {
            PassCount ++;
        }
    }



    // TODO: Init this once
    UniformDataList uniformDataList(PassCount);

    for (auto i = 0; i < PassCount; i++)
    {
        uniformDataList[i] = std::make_shared<UniformData>();
    }
    auto& mainUniform = uniformDataList[indexMap.at(CommandsType::MainPass)];

    mainUniform->clipPlane = glm::vec4(0.0, 1.0, 0.0, 100);
    mainUniform->time = getTime();
    _sceneManager->getMainCamera()->fillUniformData(*mainUniform);

    for (auto i = 1; i < PassCount; i++)
    {
        *uniformDataList[i] = *mainUniform;
    }

    for (auto i = 0u; i < _sceneManager->getLightManager()->getLightCount(); i++)
    {
        _sceneManager->getLightManager()->getLight(i)->updateViewMatrix(_sceneManager->getMainCamera()->getPosition());
    }

    for (auto l = 0; l < _sceneManager->getLightManager()->getLightCount(); l++)
    {
        _sceneManager->getLightManager()->fillUniformData(*uniformDataList[indexMap.at(CommandsType::ShadowPass) + l], l);
    }
    for (auto i = 0; i < PassCount; i++)
    {
        if (i >= indexMap.at(CommandsType::ShadowPass) && i < indexMap.at(CommandsType::ShadowPass) + _sceneManager->getLightManager()->getLightCount())
            continue;
        _sceneManager->getLightManager()->fillUniformData(*uniformDataList[i]);
    }

    if (auto water = _sceneManager->getWater())
    {
        water->getVulkanWater()->fillUniformData(*uniformDataList[indexMap.at(CommandsType::ReflectionPass)],
                                                 VulkanWater::PassType::Reflection);
        water->getVulkanWater()->fillUniformData(*uniformDataList[indexMap.at(CommandsType::RefractionPass)],
                                                 VulkanWater::PassType::Refraction);
    }


    /////// Update uniforms

    if (skybox)
        skybox->updateUniforms(uniformDataList, indexMap);
    updateNode(_sceneManager->getRootNode(), uniformDataList, indexMap);

    ///////  Submit command buffers
    if (isShadowMappingEnabled())
    {
        for (auto i = 0u; i < _sceneManager->getLightManager()->getLightCount(); i++)
        {
            if (_sceneManager->getLightManager()->getLight(i)->castShadows())
            {
                _vulkanInstance->submitCommands(
                        CommandsType::ShadowPass,
                        _sceneManager->getLightManager()->getVulkanShadowImage()->getBufferID(
                                i, _vulkanInstance->getCurrentFrameIndex()));
            }
        }
    }
    if (auto water = _sceneManager->getWater())
    {
        _vulkanInstance->submitCommands(CommandsType::ReflectionPass, BUFFER_INDEX_WATER_REFLECTION);
        _vulkanInstance->submitCommands(CommandsType::RefractionPass, BUFFER_INDEX_WATER_REFRACTION);
    }
    // TODO: Check if screen quad rendering enabled
    if (_vulkanInstance->getScreenQuad())
        _vulkanInstance->submitCommands(CommandsType::ScreenQuadPass, BUFFER_INDEX_SCREEN_QUAD);
    _vulkanInstance->submitCommands(CommandsType::MainPass, _vulkanInstance->getCurrentFrameIndex());

    _vulkanInstance->renderCommands();
}

float Engine::getTime()
{
    return _duration;
}

void Engine::updateTime()
{
    _currentTime = std::chrono::high_resolution_clock::now();
    _duration = std::chrono::duration<float, std::chrono::seconds::period>(_currentTime - _startTime).count();
}

CommandsType Engine::getPassType() const
{
    return _commandsType;
}

uint32_t Engine::getPassSubIndex() const
{
    return _passSubIndex;
}

bool Engine::isShadowMappingEnabled() const
{
    // TODO: Refactor this or remove
    return true;//_sceneManager->getShadowMap()->isEnabled();
}

bool Engine::isWaterEnabled() const
{
    return _sceneManager->getWater() != nullptr;
}

void Engine::destroyInstance()
{
    delete _engineInstance;
}

} // namespace SVE