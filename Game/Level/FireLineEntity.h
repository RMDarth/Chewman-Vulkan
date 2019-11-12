// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/Entity.h>

namespace SVE
{
class Material;
}

namespace Chewman
{

struct FireLineInfo
{
    glm::vec3 startPos;
    glm::vec3 direction;
    float percent;
    int maxParticles;
    float maxLength;
    float alpha;
};

class FireLineEntity : public SVE::Entity
{
public:
    FireLineEntity(const std::string& material, FireLineInfo startInfo);

    void updateInfo(FireLineInfo info);
    FireLineInfo& getInfo();
    void updateUniforms(SVE::UniformDataList uniformDataList) const override;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;

private:
    SVE::Material* _material = nullptr;
    uint32_t _materialIndex = 0;
    FireLineInfo _currentInfo;
};

} // namespace Chewman