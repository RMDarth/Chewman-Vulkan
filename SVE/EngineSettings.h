// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#pragma once

#include <string>

namespace SVE
{

struct EngineSettings
{
    enum class PresentMode : uint8_t
    {
        FIFO = 0,
        Mailbox,
        Immediate,
        BestAvailable
    };

    std::string applicationName = "VulkanApp";
    int gpuIndex = BEST_GPU_AVAILABLE;
    PresentMode presentMode = PresentMode::BestAvailable;
    int MSAALevel = BEST_MSAA_AVAILABLE;
    bool useValidation = false;
    bool useScreenQuad = true;
    bool initShadows = false;
    bool initWater = false;
    bool useCascadeShadowMap = false;
    bool particlesEnabled = true;

    static const int BEST_GPU_AVAILABLE;
    static const int BEST_MSAA_AVAILABLE;
};

} // namespace SVE