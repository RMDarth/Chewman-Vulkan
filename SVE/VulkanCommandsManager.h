// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>

namespace SVE
{

class VulkanCommandsManager
{
public:
    virtual void reallocateCommandBuffers() = 0;
    virtual uint32_t startRenderCommandBufferCreation(uint32_t bufferNumber, uint32_t imageIndex) = 0;
    virtual void endRenderCommandBufferCreation(uint32_t bufferIndex) = 0;
};

} // namespace SVE
