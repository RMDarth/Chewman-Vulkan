// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "SVE/ParticleSystemEntity.h"
#include "SVE/LightNode.h"

namespace Chewman
{

enum class GargoyleType : uint8_t
{
    Fire,
    Frost
};

enum class GargoyleDir : uint8_t
{
    Left,
    Up,
    Right,
    Down
};

enum class GargoyleState : uint8_t
{
    Fire,
    Rest
};

struct Gargoyle
{
    std::shared_ptr<SVE::ParticleSystemEntity> particleSystem;
    std::shared_ptr<SVE::LightNode> lightNode;

    glm::vec3 startPoint;
    glm::vec3 finalPoint;
    glm::vec3 direction;
    float totalLength;

    float currentTime = 0;
    float restTime;
    float fireTime;

    float currentLength;
    uint32_t lengthInCells;

    uint32_t row;
    uint32_t column;

    GargoyleState state = GargoyleState::Rest;
    GargoyleType type;
    GargoyleDir dir;
};

} // namespace SVE