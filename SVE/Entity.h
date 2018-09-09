// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include "Libs.h"
#include "SubmitInfo.h"

namespace SVE
{

// Base class for entities that can be attached to scene nodes
class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual SubmitInfo render() const = 0;

    const glm::mat4& getTransformation() const;
    void setTransformation(glm::mat4 transform);

protected:
    glm::mat4 _transformation = glm::mat4(1);
};

} // namespace SVE