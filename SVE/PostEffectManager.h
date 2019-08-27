// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include "Entity.h"

namespace SVE
{

class Material;
class VulkanPostEffect;

struct PostEffect
{
    ~PostEffect();
    PostEffect(PostEffect&& other) noexcept = default;

    uint32_t index;
    std::string name;
    Material* material;
    uint32_t materialIndex;
    int width;
    int height;
    std::unique_ptr<VulkanPostEffect> vulkanPostEffect;
};

class PostEffectManager
{
public:
    ~PostEffectManager();

    void addPostEffect(const std::string& materialName, std::string effectName, int width = -1, int height = -1);
    uint32_t getEffectIndex(const std::string& name);

    void createCommands(uint32_t currentFrame, uint32_t currentImage);
    void submitCommands(UniformDataList uniformDataList);

private:
    std::vector<PostEffect> _effectList;
    std::map<std::string, uint32_t> _effectMap;
};

} // namespace SVE