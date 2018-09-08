// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include "SubmitInfo.h"

namespace SVE
{

// Base class for entities that can be attached to scene nodes
class Entity
{
public:
    virtual SubmitInfo render() const = 0;
    virtual ~Entity() = default;
};

} // namespace SVE