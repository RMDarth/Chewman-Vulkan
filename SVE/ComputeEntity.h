// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Entity.h"

namespace SVE
{

class ComputeEntity : public Entity
{
public:
    virtual void applyComputeCommands(uint32_t bufferIndex, uint32_t imageIndex) const = 0;

    static void startComputeStep();
    static void finishComputeStep();

    bool isComputeEntity() const override;
};

} // namespace SVE