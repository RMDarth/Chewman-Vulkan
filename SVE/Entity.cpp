// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include <utility>
#include "Entity.h"

namespace SVE
{

const glm::mat4& Entity::getTransformation() const
{
    return _transformation;
}

void Entity::setTransformation(glm::mat4 transform)
{
    _transformation = std::move(transform);
}
} // namespace SVE