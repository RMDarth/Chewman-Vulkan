// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "GameMap.h"
#include "SVE/Engine.h"
#include "SVE/MeshManager.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include "SVE/LightNode.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{
constexpr float CellSize = 3.0f;

GameMap::GameMap()
    : _meshGenerator(CellSize)
{

}

void GameMap::LoadMap(const std::string& filename)
{
    std::ifstream fin(filename);

    fin >> _width >> _height;

    uint8_t style, waterStyle;
    std::string levelName;
    fin >> style >> waterStyle >> levelName;

    _mapNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    SVE::Engine::getInstance()->getSceneManager()->getRootNode()->attachSceneNode(_mapNode);

    _mapData.resize(_height);
    char ch;
    char line[100];
    for (auto row = 0; row < _height; ++row)
    {
        fin.getline(line, 100);
        auto curRow = _height - row - 1;
        _mapData[curRow].resize(_width);
        for (auto column = 0; column < _width; ++column)
        {
            fin >> ch;
            switch (ch)
            {
                case 'W':
                    _mapData[curRow][column] = CellType::Wall;
                    break;
                case 'L':
                    _mapData[curRow][column] = CellType::Liquid;
                    break;
                case '0':
                case 'C':
                    _mapData[curRow][column] = CellType::Floor;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    _mapData[curRow][column] = CellType::InvisibleWallWithFloor;
                    CreateGargoyle(curRow, column, ch);
                    break;
                default:
                    _mapData[curRow][column] = CellType::InvisibleWallWithFloor;
                    break;
            }
        }
    }

    fin.getline(line, 100);
    InitMeshes();
    for (auto& gargoyle : _gargoyles)
    {
        uint32_t startTime, finishTime;
        fin >> startTime >> finishTime;
        // Old trashman used weird timing, when 10 clocks are used when 15 passed
        startTime *= 1.5f;
        finishTime *= 1.5f;
        gargoyle.restTime = (float)startTime / 1000;
        gargoyle.fireTime = (float)(finishTime - startTime) / 1000;
        UpdateGargoyle(gargoyle);
    }

    fin.close();
}

void GameMap::InitMeshes()
{
    std::vector<Submesh> top;
    std::vector<Submesh> bottom;
    std::vector<Submesh> vertical;
    for (auto x = 0; x < _height; ++x)
    {
        for (auto y = 0; y < _width; ++y)
        {
            std::vector<Submesh> cellT;
            std::vector<Submesh> cellB;
            std::vector<Submesh> cellV;
            glm::vec3 position(y * CellSize, 0, -x * CellSize);
            switch (_mapData[x][y])
            {
                case CellType::Wall:
                    cellT = _meshGenerator.GenerateWall(position, ModelType::Top);
                    cellB = _meshGenerator.GenerateWall(position, ModelType::Bottom);
                    cellV = _meshGenerator.GenerateWall(position, ModelType::Vertical);
                    break;
                case CellType::Floor:
                case CellType::InvisibleWallWithFloor:
                    cellT = _meshGenerator.GenerateFloor(position, ModelType::Top);
                    cellB = _meshGenerator.GenerateFloor(position, ModelType::Bottom);
                    cellV = _meshGenerator.GenerateFloor(position, ModelType::Vertical);
                    break;
                case CellType::Liquid:
                    cellT = _meshGenerator.GenerateLiquid(position, ModelType::Top);
                    cellB = _meshGenerator.GenerateLiquid(position, ModelType::Bottom);
                    cellV = _meshGenerator.GenerateLiquid(position, ModelType::Vertical);
                    break;
            }
            top.insert(top.end(), cellT.begin(), cellT.end());
            bottom.insert(bottom.end(), cellB.begin(), cellB.end());
            vertical.insert(vertical.end(), cellV.begin(), cellV.end());
        }
    }

    auto meshSettingsT = _meshGenerator.CombineMeshes("MapT", top);
    meshSettingsT.materialName = "CeilingNormals";
    auto meshSettingsB = _meshGenerator.CombineMeshes("MapB", bottom);
    meshSettingsB.materialName = "FloorParallax";
    auto meshSettingsV = _meshGenerator.CombineMeshes("MapV", vertical);
    meshSettingsV.materialName = "WallParallax";

    auto* engine = SVE::Engine::getInstance();

    _mapMesh[0] = std::make_shared<SVE::Mesh>(meshSettingsT);
    engine->getMeshManager()->registerMesh(_mapMesh[0]);
    _mapMesh[1] = std::make_shared<SVE::Mesh>(meshSettingsB);
    engine->getMeshManager()->registerMesh(_mapMesh[1]);
    _mapMesh[2] = std::make_shared<SVE::Mesh>(meshSettingsV);
    engine->getMeshManager()->registerMesh(_mapMesh[2]);

    _mapEntity[0] = std::make_shared<SVE::MeshEntity>("MapT");
    _mapEntity[1] = std::make_shared<SVE::MeshEntity>("MapB");
    _mapEntity[2] = std::make_shared<SVE::MeshEntity>("MapV");

    //_mapNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(10, 5, 0)));
    _mapNode->attachEntity(_mapEntity[0]);
    _mapNode->attachEntity(_mapEntity[1]);
    _mapNode->attachEntity(_mapEntity[2]);
}

void GameMap::CreateGargoyle(int row, int column, char mapType)
{
    auto* engine = SVE::Engine::getInstance();

    Gargoyle gargoyle;
    gargoyle.type = mapType > '4' ? GargoyleType::Frost : GargoyleType::Fire;
    auto dirVal = (mapType - '1') % 4;
    gargoyle.dir = static_cast<GargoyleDir>(dirVal);
    gargoyle.row = row;
    gargoyle.column = column;

    // Setup root node
    auto rootPos = glm::vec3(CellSize * column, CellSize / 4, -CellSize * row);
    auto rootGargoyleNode = engine->getSceneManager()->createSceneNode();
    {
        auto mat = glm::translate(glm::mat4(1), rootPos);
        mat = glm::rotate(mat, glm::radians(-90.0f * dirVal), glm::vec3(0.0f, 1.0f, 0.0f));
        rootGargoyleNode->setNodeTransformation(mat);
    }

    // Add model
    std::shared_ptr<SVE::Entity> meshEntity = std::make_shared<SVE::MeshEntity>("gargoyle");
    meshEntity->setMaterial("FireGargoyle");
    auto gargoyleMeshNode = engine->getSceneManager()->createSceneNode();
    gargoyleMeshNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.15f, 0.0f)));
    gargoyleMeshNode->attachEntity(meshEntity);
    rootGargoyleNode->attachSceneNode(gargoyleMeshNode);

    // Add particles
    auto particlesNode = engine->getSceneManager()->createSceneNode();
    {
        auto mat = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.95f, 0.0f));
        mat = glm::rotate(mat, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mat = glm::translate(mat, glm::vec3(0.0f, 0.0f, 1.5f));
        particlesNode->setNodeTransformation(mat);
    }
    rootGargoyleNode->attachSceneNode(particlesNode);
    _mapNode->attachSceneNode(rootGargoyleNode);
    std::shared_ptr<SVE::ParticleSystemEntity> particleSystem = std::make_shared<SVE::ParticleSystemEntity>("FireParticle");
    {
        auto &emitter = particleSystem->getSettings().particleEmitter;
        emitter.minLife = 0;
        emitter.maxLife = 0;
    }
    particlesNode->attachEntity(particleSystem);
    gargoyle.particleSystem = particleSystem;

    // Add light effect
    SVE::LightSettings lightSettings {};
    lightSettings.lightType = SVE::LightType::LineLight;
    lightSettings.castShadows = false;
    lightSettings.diffuseStrength = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightSettings.specularStrength = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    lightSettings.ambientStrength = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    lightSettings.shininess = 16;
    lightSettings.constAtten = 1.0f * 0.8f;
    lightSettings.linearAtten = 0.35f * 0.05f;
    lightSettings.quadAtten = 0.44f * 0.05f;
    auto lightManager = engine->getSceneManager()->getLightManager();
    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings, lightManager->getLightCount());
    lightManager->setLight(lightNode, lightManager->getLightCount());
    rootGargoyleNode->attachSceneNode(lightNode);
    gargoyle.startPoint = rootPos;
    gargoyle.lightNode = lightNode;

    _gargoyles.push_back(std::move(gargoyle));
}

void GameMap::UpdateGargoyle(Gargoyle &gargoyle)
{
    int speedX = 0;
    int speedY = 0;
    int x = gargoyle.row;
    int y = gargoyle.column;
    switch (gargoyle.dir)
    {
        case GargoyleDir::Left:
            speedY = -1;
            break;
        case GargoyleDir::Up:
            speedX = 1;
            break;
        case GargoyleDir::Right:
            speedY = 1;
            break;
        case GargoyleDir::Down:
            speedX = -1;
            break;
    }
    bool stop = false;
    float length = 0;
    while(!stop)
    {
        x += speedX;
        y += speedY;
        ++length;

        switch (_mapData[x][y])
        {
            case CellType::Wall:
            case CellType::InvisibleWallWithFloor:
            case CellType::InvisibleWallEmpty:
                stop = true;
                break;
        }
        if (x >= _width || x < 0 || y >= _height || y < 0)
            stop = true;
    }
    length-= 1.5f;
    gargoyle.lengthInCells = length;
    auto& emitter = gargoyle.particleSystem->getSettings().particleEmitter;
    emitter.minSpeed = CellSize * length / gargoyle.fireTime;
    emitter.maxSpeed = emitter.minSpeed;

    x-=speedX;
    y-=speedY;
    gargoyle.finalPoint = glm::vec3(CellSize * y, CellSize / 4, -CellSize * x);
    auto distance = gargoyle.finalPoint - gargoyle.startPoint;
    gargoyle.direction = glm::normalize(distance);
    gargoyle.totalLength = glm::length(distance);
}

void GameMap::Update(float time)
{
    if (time <= 0.0)
        return;

    auto updateGargoyleParticles = [](Gargoyle& gargoyle, float life, float alphaChanger, float lifeDrain)
    {
        auto& emitter = gargoyle.particleSystem->getSettings().particleEmitter;
        auto& affector = gargoyle.particleSystem->getSettings().particleAffector;
        emitter.minLife = life;
        emitter.maxLife = life;
        affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, alphaChanger);
        affector.lifeDrain = lifeDrain;
    };

    for (auto& gargoyle : _gargoyles)
    {
        gargoyle.currentTime += time;
        switch (gargoyle.state)
        {
            case GargoyleState::Fire:
            {
                if (gargoyle.currentTime > gargoyle.fireTime - 0.2f)
                {
                    gargoyle.state = GargoyleState::Rest;
                    gargoyle.currentTime = 0;
                    gargoyle.currentLength = 0;
                    updateGargoyleParticles(gargoyle, 0.0f, -3.5f, 5.0f);
                } else {
                    float finishPercent = gargoyle.currentTime / gargoyle.fireTime;
                    gargoyle.currentLength = (float)gargoyle.lengthInCells * finishPercent;

                    gargoyle.lightNode->getLightSettings().lightType = SVE::LightType::LineLight;
                    gargoyle.lightNode->getLightSettings().secondPoint = gargoyle.startPoint + gargoyle.direction * gargoyle.totalLength * finishPercent;
                }
                break;
            }
            case GargoyleState::Rest:
            {
                if (gargoyle.currentTime > 0.2f)
                {
                    gargoyle.lightNode->getLightSettings().lightType = SVE::LightType::None;
                    gargoyle.lightNode->getLightSettings().secondPoint = gargoyle.startPoint;
                }
                if (gargoyle.currentTime > gargoyle.restTime + 0.2f)
                {
                    gargoyle.state = GargoyleState::Fire;
                    gargoyle.currentTime = 0;
                    gargoyle.currentLength = 0;
                    updateGargoyleParticles(gargoyle, 10.0f, 0.0f, 0.0f);
                }
                break;
            }
        }
    }

}

} // namespace Chewman