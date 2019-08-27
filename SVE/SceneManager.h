// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "SceneNode.h"
#include "CameraNode.h"
#include <memory>

namespace SVE
{
class LightManager;
class LightNode;
class LightSettings;
class ParticleSystemManager;
class ParticleSystemEntity;
class ParticleSystemSettings;
class Skybox;
class ShadowMap;
class Water;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    // TODO: Add get node by name
    std::shared_ptr<SceneNode> getRootNode();

    std::shared_ptr<SceneNode> createSceneNode(std::string name = "");

    std::shared_ptr<CameraNode> createMainCamera();
    std::shared_ptr<CameraNode> getMainCamera();
    void setMainCamera(std::shared_ptr<CameraNode> cameraEntity);

    std::shared_ptr<LightNode> createLight(LightSettings lightSettings);
    LightManager* getLightManager();

    void setSkybox(const std::string& materialName);
    void setSkybox(std::shared_ptr<Skybox> skybox);
    std::shared_ptr<Skybox> getSkybox();

    std::shared_ptr<Water> createWater(float height);
    std::shared_ptr<Water> getWater();

private:
    bool _recreateCommandBuffers = true;
    std::shared_ptr<SceneNode> _root;
    std::shared_ptr<CameraNode> _mainCamera;
    std::shared_ptr<Skybox> _skybox;
    std::shared_ptr<Water> _water;

    std::unique_ptr<LightManager> _lightManager;

};

} // namespace SVE