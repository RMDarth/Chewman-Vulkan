// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "SceneNode.h"
#include <memory>

namespace SVE
{

class SceneManager
{
public:
    SceneManager();

    std::shared_ptr<SceneNode> getRootNode();

private:
    std::shared_ptr<SceneNode> _root;
};

} // namespace SVE