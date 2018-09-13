// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "SceneNode.h"
#include "CameraNode.h"
#include <memory>

namespace SVE
{
class LightNode;
class LightSettings;

class SceneManager
{
public:
    SceneManager();

    std::shared_ptr<SceneNode> getRootNode();

    std::shared_ptr<SceneNode> createSceneNode(std::string name = "");

    std::shared_ptr<CameraNode> createMainCamera();
    std::shared_ptr<CameraNode> getMainCamera();
    void setMainCamera(std::shared_ptr<CameraNode> cameraEntity);

    // TODO: Support multiple lights
    std::shared_ptr<LightNode> createLight(LightSettings lightSettings);
    std::shared_ptr<LightNode> getLight();

    void queueCommandBuffersUpdate();
    void dequeueCommandBufferUpdate();
    bool isCommandBufferUpdateQueued();

private:
    bool _recreateCommandBuffers = true;
    std::shared_ptr<SceneNode> _root;
    std::shared_ptr<CameraNode> _mainCamera;
    std::shared_ptr<LightNode> _lightNode;
};

} // namespace SVE