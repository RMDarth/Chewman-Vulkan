// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "SceneNode.h"
#include "CameraNode.h"
#include <memory>

namespace SVE
{

class SceneManager
{
public:
    SceneManager();

    std::shared_ptr<SceneNode> getRootNode();

    std::shared_ptr<SceneNode> createSceneNode(std::string name = "");

    std::shared_ptr<CameraNode> createMainCamera();
    std::shared_ptr<CameraNode> getMainCamera();
    void setMainCamera(std::shared_ptr<CameraNode> cameraEntity);

    void queueCommandBuffersUpdate();
    void dequeueCommandBufferUpdate();
    bool isCommandBufferUpdateQueued();

private:
    bool _recreateCommandBuffers = true;
    std::shared_ptr<SceneNode> _root;
    std::shared_ptr<CameraNode> _mainCamera;
};

} // namespace SVE