// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanDirectShadowMap.h"
#include "VulkanWater.h"
#include "VulkanScreenQuad.h"
#include "VulkanException.h"
#include "VulkanMaterial.h"
#include "MaterialManager.h"
#include "SceneManager.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "LightManager.h"
#include "ParticleSystemManager.h"
#include "PostEffectManager.h"
#include "ResourceManager.h"
#include "FontManager.h"
#include "OverlayManager.h"
#include "Entity.h"
#include "Skybox.h"
#include "ShadowMap.h"
#include "Water.h"
#include "Utils.h"
#include "ComputeEntity.h"
#include <chrono>
#include <utility>

namespace SVE
{

namespace
{

enum class PassStage : uint8_t
{
    Start,
    Instanced,
    Deferred
};

} // anon namespace

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
    , _particleSystemManager(std::make_unique<ParticleSystemManager>())
    , _postEffectManager(std::make_unique<PostEffectManager>())
    , _fontManager(std::make_unique<FontManager>())
    , _overlayManager(std::make_unique<OverlayManager>())
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
    _postEffectManager.reset();
    _fontManager.reset();
    _overlayManager.reset();
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

ParticleSystemManager* Engine::getParticleSystemManager()
{
    return _particleSystemManager.get();
}

PostEffectManager* Engine::getPostEffectManager()
{
    return _postEffectManager.get();
}

FontManager* Engine::getFontManager()
{
    return _fontManager.get();
}

OverlayManager* Engine::getOverlayManager()
{
    return _overlayManager.get();
}

void Engine::resizeWindow()
{
    _vulkanInstance->resizeWindow();
    _materialManager->resetPipelines();

    _sceneManager->getMainCamera()->setAspectRatio(
            (float)_vulkanInstance->getExtent().width / _vulkanInstance->getExtent().height);
}

glm::ivec2 Engine::getRenderWindowSize()
{
    int w, h;
    SDL_GetWindowSize(_vulkanInstance->getWindow(), &w, &h);
    return glm::ivec2(w, h);
}

void Engine::finishRendering()
{
    _vulkanInstance->finishRendering();
}

void createNodeStageDrawCommands(const std::shared_ptr<SceneNode>& node, uint32_t bufferIndex, uint32_t imageIndex, PassStage stage)
{
    auto passType = Engine::getInstance()->getPassType();
    for (auto& entity : node->getAttachedEntities())
    {
        bool isRenderLast = entity->isRenderLast();
        bool isRenderDepth = entity->isRenderToDepth();
        bool isInstanced = entity->isInstanceRendering();

        if (!isRenderDepth && passType == CommandsType::ScreenQuadDepthPass)
        {
            continue;
        }

        switch (stage)
        {
            case PassStage::Start:
                if (!isRenderLast && !isInstanced)
                    entity->applyDrawingCommands(bufferIndex, imageIndex);
                break;
            case PassStage::Instanced:
                if (!isRenderLast && isInstanced)
                {
                    entity->updateInstanceBuffers();
                    entity->applyDrawingCommands(bufferIndex, imageIndex);
                }
                break;
            case PassStage::Deferred:
                if (isRenderLast)
                    entity->applyDrawingCommands(bufferIndex, imageIndex);
                break;
        }
    }

    for (auto& child : node->getChildren())
    {
        createNodeStageDrawCommands(child, bufferIndex, imageIndex, stage);
    }
}

void createNodeDrawCommands(const std::shared_ptr<SceneNode>& node, uint32_t bufferIndex, uint32_t imageIndex)
{
    createNodeStageDrawCommands(node, bufferIndex, imageIndex, PassStage::Start);
    createNodeStageDrawCommands(node, bufferIndex, imageIndex, PassStage::Instanced);
    createNodeStageDrawCommands(node, bufferIndex, imageIndex, PassStage::Deferred);
}

void createNodeComputeCommands(const std::shared_ptr<SceneNode>& node, uint32_t bufferIndex, uint32_t imageIndex)
{

    for (auto& entity : node->getAttachedEntities())
    {
        if (entity->isComputeEntity())
        {
            auto* computeEntity = static_cast<ComputeEntity*>(entity.get());
            computeEntity->applyComputeCommands(bufferIndex, imageIndex);
        }
    }

    for (auto& child : node->getChildren())
    {
        createNodeComputeCommands(child, bufferIndex, imageIndex);
    }
}

void setFrameNumber(const std::shared_ptr<SceneNode>& node, uint64_t frameId)
{
    node->setCurrentFrame(frameId);
    for (auto& child : node->getChildren())
    {
        setFrameNumber(child, frameId);
    }
}

void updateNode(const std::shared_ptr<SceneNode>& node, UniformDataList& uniformDataList)
{
    auto oldModel = uniformDataList[0]->model;
    uniformDataList[0]->model *= node->getNodeTransformation();
    for (auto i = 1u; i < uniformDataList.size(); i++)
        uniformDataList[i]->model = uniformDataList[0]->model;

    // update uniforms
    for (auto& entity : node->getAttachedEntities())
    {
        entity->updateUniforms(uniformDataList);
    }

    for (auto& child : node->getChildren())
    {
        updateNode(child, uniformDataList);
    }

    for (auto i = 0u; i < uniformDataList.size(); i++)
        uniformDataList[i]->model = oldModel;
}

void Engine::renderFrame()
{
    _vulkanInstance->waitAvailableFramebuffer();
    updateTime();
    renderFrameImpl();
}

void Engine::renderFrame(float deltaTime)
{
    _vulkanInstance->waitAvailableFramebuffer();
    _prevTime = _currentTime;
    _currentTime += std::chrono::duration<int, std::chrono::microseconds::period>(static_cast<int>(deltaTime * 1000000));
    _duration = std::chrono::duration<float, std::chrono::seconds::period>(_currentTime - _startTime).count();
    _deltaTime = deltaTime;

    renderFrameImpl();
}

void Engine::renderFrameImpl()
{
    ++_frameId;
    auto skybox = _sceneManager->getSkybox();
    auto currentFrame = _vulkanInstance->getCurrentFrameIndex();
    auto currentImage = _vulkanInstance->getCurrentImageIndex();

    ////// update command buffers

    _vulkanInstance->reallocateCommandBuffers();
    setFrameNumber(_sceneManager->getRootNode(), _frameId);

    ComputeEntity::startComputeStep();
    createNodeComputeCommands(_sceneManager->getRootNode(), BUFFER_INDEX_COMPUTE_PARTICLES, currentImage);
    ComputeEntity::finishComputeStep();

    _commandsType = CommandsType::ShadowPassDirectLight;
    _sceneManager->getLightManager()->setCurrentFrame(_frameId);
    if (auto directLight = _sceneManager->getLightManager()->getDirectionLight())
    {
        if (auto sunLightShadowMap = _sceneManager->getLightManager()->getDirectLightShadowMap())
        {
            sunLightShadowMap->getVulkanShadowMap()->reallocateCommandBuffers();

            auto bufferIndex =
                    sunLightShadowMap->getVulkanShadowMap()->startRenderCommandBufferCreation(
                            _vulkanInstance->getCurrentFrameIndex(),
                            _vulkanInstance->getCurrentImageIndex());
            createNodeDrawCommands(_sceneManager->getRootNode(), bufferIndex, currentImage);
            sunLightShadowMap->getVulkanShadowMap()->endRenderCommandBufferCreation(
                    _vulkanInstance->getCurrentFrameIndex());
        }
    }

    _commandsType = CommandsType::ShadowPassPointLights;
    if (auto pointLightShadowMap = _sceneManager->getLightManager()->getPointLightShadowMap())
    {
        pointLightShadowMap->getVulkanShadowMap()->reallocateCommandBuffers();

        auto bufferIndex =
                pointLightShadowMap->getVulkanShadowMap()->startRenderCommandBufferCreation(
                        _vulkanInstance->getCurrentFrameIndex(),
                        _vulkanInstance->getCurrentImageIndex());
        createNodeDrawCommands(_sceneManager->getRootNode(), bufferIndex, currentImage);
        pointLightShadowMap->getVulkanShadowMap()->endRenderCommandBufferCreation(
                _vulkanInstance->getCurrentFrameIndex());
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
        _commandsType = CommandsType::ScreenQuadDepthPass;
        screenQuad->reallocateCommandBuffers(VulkanScreenQuad::Depth);
        screenQuad->startRenderCommandBufferCreation(VulkanScreenQuad::Depth);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD_DEPTH, currentImage, PassStage::Start);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD_DEPTH, currentImage, PassStage::Instanced);
        screenQuad->endRenderCommandBufferCreation(VulkanScreenQuad::Depth);

        _commandsType = CommandsType::ScreenQuadPass;
        screenQuad->reallocateCommandBuffers(VulkanScreenQuad::Normal);
        screenQuad->startRenderCommandBufferCreation(VulkanScreenQuad::Normal);
        if (skybox)
            skybox->applyDrawingCommands(BUFFER_INDEX_SCREEN_QUAD, currentImage);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD, currentImage, PassStage::Start);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD, currentImage, PassStage::Instanced);
        screenQuad->endRenderCommandBufferCreation(VulkanScreenQuad::Normal);

        _commandsType = CommandsType::ScreenQuadMRTPass;
        screenQuad->reallocateCommandBuffers(VulkanScreenQuad::MRT);
        screenQuad->startRenderCommandBufferCreation(VulkanScreenQuad::MRT);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD_MRT, currentImage, PassStage::Start);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD_MRT, currentImage, PassStage::Instanced);
        screenQuad->endRenderCommandBufferCreation(VulkanScreenQuad::MRT);

        _commandsType = CommandsType::ScreenQuadLatePass;
        screenQuad->reallocateCommandBuffers(VulkanScreenQuad::Late);
        screenQuad->startRenderCommandBufferCreation(VulkanScreenQuad::Late);
        createNodeStageDrawCommands(_sceneManager->getRootNode(), BUFFER_INDEX_SCREEN_QUAD_LATE, currentImage, PassStage::Deferred);
        screenQuad->endRenderCommandBufferCreation(VulkanScreenQuad::Late);

        _commandsType = CommandsType::PostEffectPasses;
        _engineInstance->getPostEffectManager()->createCommands(currentFrame, currentImage);
    }

    _commandsType = CommandsType::MainPass;
    _vulkanInstance->startRenderCommandBufferCreation();
    if (auto* screenQuad = _vulkanInstance->getScreenQuad())
    {
        auto index = _engineInstance->getMaterialManager()->getMaterial("ScreenQuad")->getVulkanMaterial()->getInstanceForEntity(nullptr);
        _materialManager->getMaterial("ScreenQuad")->getVulkanMaterial()->applyDrawingCommands(currentFrame, currentImage, index);
        auto commandBuffer = _vulkanInstance->getCommandBuffer(currentFrame);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);

        // Draw GUI
        _overlayManager->applyDrawingCommands(currentFrame, currentImage);
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

    // TODO: Init this once
    UniformDataList uniformDataList(PassCount);

    for (auto i = 0; i < PassCount; i++)
    {
        uniformDataList[i] = std::make_shared<UniformData>();
    }
    auto& mainUniform = uniformDataList[toInt(CommandsType::MainPass)];

    mainUniform->clipPlane = glm::vec4(0.0, 1.0, 0.0, 100);
    mainUniform->time = getTime();
    mainUniform->deltaTime = getDeltaTime();
    mainUniform->imageSize = glm::ivec4(getRenderWindowSize(), 0, 0);
    _sceneManager->getMainCamera()->fillUniformData(*mainUniform);

    for (auto i = 1; i < PassCount; i++)
    {
        *uniformDataList[i] = *mainUniform;
    }

    _sceneManager->getLightManager()->getDirectionLight()->updateViewMatrix(_sceneManager->getMainCamera()->getPosition(),
                                                                            _sceneManager->getMainCamera()->getDirection());
    _sceneManager->getLightManager()->fillUniformData(*uniformDataList[toInt(CommandsType::ShadowPassDirectLight)], LightType::SunLight);
    _sceneManager->getLightManager()->fillUniformData(*uniformDataList[toInt(CommandsType::ShadowPassPointLights)], LightType::ShadowPointLight);
    for (auto i = 0u; i < PassCount; i++)
    {
        if (i == toInt(CommandsType::ShadowPassDirectLight) || i == toInt(CommandsType::ShadowPassPointLights))
            continue;
        _sceneManager->getLightManager()->fillUniformData(*uniformDataList[i]);
    }

    if (auto water = _sceneManager->getWater())
    {
        water->getVulkanWater()->fillUniformData(*uniformDataList[toInt(CommandsType::ReflectionPass)],
                                                 VulkanWater::PassType::Reflection);
        water->getVulkanWater()->fillUniformData(*uniformDataList[toInt(CommandsType::RefractionPass)],
                                                 VulkanWater::PassType::Refraction);
    }

    /////// Update uniforms

    if (skybox)
        skybox->updateUniforms(uniformDataList);
    updateNode(_sceneManager->getRootNode(), uniformDataList);
    _overlayManager->updateUniforms(uniformDataList);

    ///////  Submit command buffers to queue
    //if (particleSystemManager)
    {
        // TODO: Use special compute queue instead of graphics queue for compute shader (they can be different)
        _vulkanInstance->submitCommands(CommandsType::ComputeParticlesPass, BUFFER_INDEX_COMPUTE_PARTICLES);
    }
    if (isShadowMappingEnabled())
    {
        if (_sceneManager->getLightManager()->getDirectLightShadowMap())
        {
            _vulkanInstance->submitCommands(
                    CommandsType::ShadowPassDirectLight,
                    BUFFER_INDEX_SHADOWMAP_SUN + _vulkanInstance->getCurrentFrameIndex());
        }

        if (_sceneManager->getLightManager()->getPointLightShadowMap())
        {
            _vulkanInstance->submitCommands(
                    CommandsType::ShadowPassPointLights,
                    BUFFER_INDEX_SHADOWMAP_POINT + _vulkanInstance->getCurrentFrameIndex());
        }
    }
    if (auto water = _sceneManager->getWater())
    {
        _vulkanInstance->submitCommands(CommandsType::ReflectionPass, BUFFER_INDEX_WATER_REFLECTION);
        _vulkanInstance->submitCommands(CommandsType::RefractionPass, BUFFER_INDEX_WATER_REFRACTION);
    }

    if (_vulkanInstance->getScreenQuad())
    {
        _vulkanInstance->submitCommands(CommandsType::ScreenQuadDepthPass, BUFFER_INDEX_SCREEN_QUAD_DEPTH);
        _vulkanInstance->submitCommands(CommandsType::ScreenQuadPass, BUFFER_INDEX_SCREEN_QUAD);
        _vulkanInstance->submitCommands(CommandsType::ScreenQuadMRTPass, BUFFER_INDEX_SCREEN_QUAD_MRT);
        _vulkanInstance->submitCommands(CommandsType::ScreenQuadLatePass, BUFFER_INDEX_SCREEN_QUAD_LATE);
        _postEffectManager->submitCommands(uniformDataList);
    }
    _vulkanInstance->submitCommands(CommandsType::MainPass, _vulkanInstance->getCurrentFrameIndex());

    _vulkanInstance->renderCommands();
}

float Engine::getTime()
{
    return _duration;
}

float Engine::getDeltaTime()
{
    return _deltaTime;
}

void Engine::updateTime()
{
    _prevTime = _currentTime;
    _currentTime = std::chrono::high_resolution_clock::now();
    _duration = std::chrono::duration<float, std::chrono::seconds::period>(_currentTime - _startTime).count();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(_currentTime - _prevTime).count();
}

CommandsType Engine::getPassType() const
{
    return _commandsType;
}

bool Engine::isShadowMappingEnabled() const
{
    // TODO: Refactor this or remove
    return true;//_sceneManager->getShadowMap()->isEnabled();
}

const EngineSettings& Engine::getEngineSettings() const
{
    return _vulkanInstance->getEngineSettings();
}

bool Engine::isWaterEnabled() const
{
    return _sceneManager->getWater() != nullptr;
}

void Engine::destroyInstance()
{
    delete _engineInstance;
    _engineInstance = nullptr;
}

} // namespace SVE