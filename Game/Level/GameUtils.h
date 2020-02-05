// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <random>
#include <memory>
#include <glm/vec3.hpp>
#include "Game/GameSettings.h"
#include "MapTraveller.h"

namespace SVE
{
class LightNode;
class Engine;
}

namespace Chewman
{

enum class SunLightType : uint8_t
{
    Day,
    Night
};

struct GlmHash
{
    size_t operator()(const glm::ivec2& v)const
    {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y);
    }
};

std::mt19937& getRandomEngine();
glm::vec3 getWorldPos(int row, int column, float y = 0.0f);
std::shared_ptr<SVE::LightNode> addEnemyLightEffect(SVE::Engine* engine, float height = 1.5f);
bool isAntiDirection(MoveDirection curDir, MoveDirection newDir);
bool isOrthogonalDirection(MoveDirection curDir, MoveDirection newDir);
void setSunLight(SunLightType sunLightType);

glm::vec3 getCameraPos(CameraStyle cameraStyle);
glm::vec3 getDeathCameraPos(CameraStyle cameraStyle);

glm::vec4 getCeilingMaterialDiffuse(uint16_t style, bool isNight);
glm::vec4 getFloorMaterialDiffuse(uint16_t style, bool isNight);

} // namespace Chewman