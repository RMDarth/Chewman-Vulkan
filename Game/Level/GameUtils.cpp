// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameUtils.h"
#include "GameMapDefs.h"
#include "SVE/Engine.h"
#include "SVE/LightNode.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <Game/Game.h>


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

std::shared_ptr<SVE::LightNode> addEnemyLightEffect(SVE::Engine* engine, float height)
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

    if (Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::Simple)
        lightSettings.isSimple = true;

    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings);
    lightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, height, 0)));

    return lightNode;
}

bool isAntiDirection(MoveDirection curDir, MoveDirection newDir)
{
    return curDir != MoveDirection::None && curDir != newDir && static_cast<uint8_t>(curDir) % 2 == static_cast<uint8_t>(newDir) % 2;
}

void setSunLight(SunLightType sunLightType)
{
    auto sunLight = SVE::Engine::getInstance()->getSceneManager()->getLightManager()->getDirectionLight();

    if (sunLightType == SunLightType::Day)
    {
        sunLight->getLightSettings().ambientStrength = {0.2f, 0.2f, 0.2f, 1.0f};
        sunLight->getLightSettings().diffuseStrength = {1.0f, 1.0f, 1.0f, 1.0f};
        sunLight->getLightSettings().specularStrength = {0.5f, 0.5f, 0.5f, 1.0f};
        sunLight->setNodeTransformation(
                glm::translate(glm::mat4(1), glm::vec3(80, 80, -80)));

        sunLight->getLightSettings().castShadows = Game::getInstance()->getGraphicsManager().getSettings().useShadows;
    } else {
        float brightness = Game::getInstance()->getGameSettingsManager().getSettings().brightness;
        brightness = 2 * (brightness - 0.5f);

        float diffuse = 0.15f + brightness * 0.15f;
        float ambient = 0.08f + brightness * 0.04f;
        float specular = 0.08f + brightness * 0.08f;

        sunLight->getLightSettings().ambientStrength = {ambient, ambient, ambient, 1.0f};
        sunLight->getLightSettings().diffuseStrength = {diffuse, diffuse, diffuse, 1.0f};
        sunLight->getLightSettings().specularStrength = {specular, specular, specular, 1.0f};
        sunLight->setNodeTransformation(
                glm::translate(glm::mat4(1), glm::vec3(-20, 80, 80)));

        sunLight->getLightSettings().castShadows = false;
    }
}

} // namespace Chewman