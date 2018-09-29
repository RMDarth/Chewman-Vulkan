// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "SceneManager.h"
#include "LightNode.h"
#include "ShadowMap.h"
#include "Skybox.h"
#include "Water.h"

namespace SVE
{

SceneManager::SceneManager()
{
    _root = std::make_shared<SceneNode>("root");
}

std::shared_ptr<SceneNode> SceneManager::getRootNode()
{
    return _root;
}

std::shared_ptr<CameraNode> SceneManager::createMainCamera()
{
    _mainCamera = std::make_shared<CameraNode>();
    _root->attachSceneNode(_mainCamera);

    return _mainCamera;
}

std::shared_ptr<CameraNode> SceneManager::getMainCamera()
{
    return _mainCamera;
}

void SceneManager::setMainCamera(std::shared_ptr<CameraNode> cameraEntity)
{
    _mainCamera = cameraEntity;
}

std::shared_ptr<LightNode> SceneManager::createLight(LightSettings lightSettings)
{
    _lightNode = std::make_shared<LightNode>(lightSettings);
    _root->attachSceneNode(_lightNode);

    return _lightNode;
}

std::shared_ptr<LightNode> SceneManager::getLight()
{
    return _lightNode;
}

std::shared_ptr<SceneNode> SceneManager::createSceneNode(std::string name)
{
    return std::make_shared<SceneNode>(name);
}

void SceneManager::setSkybox(const std::string& materialName)
{
    _skybox = std::make_shared<Skybox>(materialName);
}

void SceneManager::setSkybox(std::shared_ptr<Skybox> skybox)
{
    _skybox = std::move(skybox);
}

std::shared_ptr<Skybox> SceneManager::getSkybox()
{
    return _skybox;
}

void SceneManager::setShadowMap(std::shared_ptr<ShadowMap> shadowMap)
{
    _shadowmap = std::move(shadowMap);
}

void SceneManager::initShadowMap()
{
    _shadowmap = std::make_shared<ShadowMap>();
}

void SceneManager::enableShadowMap()
{
    _shadowmap->enableShadowMap();
}

std::shared_ptr<ShadowMap> SceneManager::getShadowMap()
{
    return _shadowmap;
}

std::shared_ptr<Water> SceneManager::createWater(float height)
{
    _water = std::make_shared<Water>(height);
    return _water;
}

std::shared_ptr<Water> SceneManager::getWater()
{
    return _water;
}

void SceneManager::queueCommandBuffersUpdate()
{
    _recreateCommandBuffers = true;
}

void SceneManager::dequeueCommandBufferUpdate()
{
    _recreateCommandBuffers = false;
}

bool SceneManager::isCommandBufferUpdateQueued()
{
    return _recreateCommandBuffers;
}

} // namespace SVE