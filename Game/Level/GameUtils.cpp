// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameUtils.h"
#include "GameMapDefs.h"
#include "SVE/Engine.h"
#include "SVE/LightNode.h"
#include <glm/gtc/matrix_transform.hpp>


namespace Chewman
{

std::mt19937& getRandomEngine()
{
    static std::mt19937 mt(std::random_device{}());
    return mt;
}

glm::vec3 getWorldPos(int row, int column, float y)
{
    return glm::vec3(CellSize * column, y, -CellSize * row);
}

std::shared_ptr<SVE::LightNode> addEnemyLightEffect(SVE::Engine* engine)
{
    SVE::LightSettings lightSettings {};
    lightSettings.lightType = SVE::LightType::PointLight;
    lightSettings.castShadows = false;
    lightSettings.diffuseStrength = glm::vec4(1.0);
    lightSettings.specularStrength = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    lightSettings.ambientStrength = { 0.2f, 0.2f, 0.2f, 1.0f };
    lightSettings.shininess = 16;
    lightSettings.constAtten = 1.0f * 1.8f;
    lightSettings.linearAtten = 0.35f * 0.25f;
    lightSettings.quadAtten = 0.44f * 0.25f;
    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings);
    lightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 1.5, 0)));

    return lightNode;
}

bool isAntiDirection(MoveDirection curDir, MoveDirection newDir)
{
    return curDir != MoveDirection::None && curDir != newDir && static_cast<uint8_t>(curDir) % 2 == static_cast<uint8_t>(newDir) % 2;
}

} // namespace Chewman