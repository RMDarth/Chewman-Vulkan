// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "SceneManager.h"

namespace SVE
{

SceneManager::SceneManager()
{
    _root = std::make_shared<SceneNode>("Root");
}

std::shared_ptr<SceneNode> SceneManager::getRootNode()
{
    return _root;
}
} // namespace SVE