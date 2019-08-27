// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ComputeEntity.h"
#include "VulkanComputeEntity.h"

namespace SVE
{

void ComputeEntity::startComputeStep()
{
    VulkanComputeEntity::startComputeStep();
}

void ComputeEntity::finishComputeStep()
{
    VulkanComputeEntity::finishComputeStep();
}

bool ComputeEntity::isComputeEntity() const
{
    return true;
}

} // namespace SVE