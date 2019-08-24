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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Chewman
{
namespace
{
constexpr float CellSize = 3.0f;

glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest)
{
    start = normalize(start);
    dest = normalize(dest);

    float cosTheta = dot(start, dest);
    glm::vec3 rotationAxis;

    rotationAxis = cross(start, dest);

    float s = sqrt( (1+cosTheta)*2 );
    float invs = 1 / s;

    return glm::quat(
            s * 0.5f,
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs
    );
}

SVE::MeshSettings constructPlane(std::string name, glm::vec3 center, float width, float height, glm::vec3 normal)
{
    std::vector<glm::vec3> points =
            {
                    {-1.0f,  0.0f,  1.0f},
                    {-1.0f,  0.0f, -1.0f},
                    {1.0f,   0.0f, -1.0f},
                    {1.0f,   0.0f, -1.0f},
                    {1.0f,   0.0f, 1.0f},
                    {-1.0f,  0.0f, 1.0f},
            };
    std::vector<glm::vec2> texCoords =
            {
                    {0, 0},
                    {0, 1},
                    {1, 1},
                    {1, 1},
                    {1, 0},
                    {0, 0}
            };
    std::vector<uint32_t> indexes;
    std::vector<glm::vec3> normals(6, normal);
    std::vector<glm::vec3> colors(6, glm::vec3(1.0f, 1.0f, 1.0f));

    uint32_t currentIndex = 0;
    for (auto& point : points)
    {
        auto mat = glm::toMat4(rotationBetweenVectors(glm::vec3(0.0, 1.0, 0.0), normal));
        mat = mat * glm::scale(glm::mat4(1), glm::vec3(width, 0, height));
        point = glm::vec3(mat * glm::vec4(point, 1.0f));
        point += center;

        indexes.push_back(currentIndex++);
    }

    SVE::MeshSettings settings {};

    settings.name = std::move(name);
    settings.vertexPosData = std::move(points);
    settings.vertexTexData = std::move(texCoords);
    settings.vertexNormalData = std::move(normals);
    settings.vertexColorData = std::move(colors);
    settings.indexData = std::move(indexes);
    settings.boneNum = 0;

    return settings;
}

glm::vec3 getWorldPos(int row, int column, float y = 0.0f)
{
    return glm::vec3(CellSize * column, y, -CellSize * row);
}

} // anon namespace

GameMap::GameMap()
    : _meshGenerator(CellSize)
{
    initTeleportMesh();
}

void GameMap::loadMap(const std::string& filename)
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
    int nextIsRotation = 0;
    for (auto row = 0; row < _height; ++row)
    {
        fin.getline(line, 100);
        auto curRow = _height - row - 1;
        _mapData[curRow].resize(_width);
        for (auto column = 0; column < _width; ++column)
        {
            fin >> ch;
            _mapData[curRow][column] = {};
            if (nextIsRotation)
            {
                nextIsRotation--;
                _mapData[curRow][column].cellType = CellType::InvisibleWallWithFloor;
                continue;
            }
            switch (ch)
            {
                case 'W':
                    _mapData[curRow][column].cellType = CellType::Wall;
                    break;
                case 'L':
                    _mapData[curRow][column].cellType = CellType::Liquid;
                    break;
                case '0':
                    _mapData[curRow][column].cellType = CellType::Floor;
                    break;
                case 'C':
                    _mapData[curRow][column].coin = createCoin(curRow, column);
                    _mapData[curRow][column].cellType = CellType::Floor;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    _mapData[curRow][column].cellType = CellType::InvisibleWallWithFloor;
                    createGargoyle(curRow, column, ch);
                    break;
                case 'r':
                case 'g':
                case 'v':
                case 'y':
                    _mapData[curRow][column].cellType = CellType::Floor;
                    createTeleport(curRow, column, ch);
                    break;
                case 'J':
                case 'D':
                case 'V':
                    nextIsRotation = ch == 'D' ? 2 : 1;
                    _mapData[curRow][column].cellType = CellType::InvisibleWallWithFloor;
                    break;
                default:
                    _mapData[curRow][column].cellType = CellType::Floor;
                    break;
            }
        }
    }

    fin.getline(line, 100);
    initMeshes();
    for (auto& gargoyle : _gargoyles)
    {
        uint32_t startTime, finishTime;
        fin >> startTime >> finishTime;
        // Old trashman used weird timing, when 10 clocks are used when 15 passed
        startTime *= 1.5f;
        finishTime *= 1.5f;
        gargoyle.restTime = (float)startTime / 1000;
        gargoyle.fireTime = (float)(finishTime - startTime) / 1000;
        finalizeGargoyle(gargoyle);
    }

    fin.close();
}

void GameMap::initMeshes()
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
            switch (_mapData[x][y].cellType)
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

void GameMap::createGargoyle(int row, int column, char mapType)
{
    auto* engine = SVE::Engine::getInstance();

    Gargoyle gargoyle;
    gargoyle.type = mapType > '4' ? GargoyleType::Frost : GargoyleType::Fire;
    auto dirVal = (mapType - '1') % 4;
    gargoyle.dir = static_cast<GargoyleDir>(dirVal);
    gargoyle.row = row;
    gargoyle.column = column;

    // Setup root node
    auto rootPos = getWorldPos(row, column, CellSize / 4);
    auto rootGargoyleNode = engine->getSceneManager()->createSceneNode();
    {
        auto mat = glm::translate(glm::mat4(1), rootPos);
        mat = glm::rotate(mat, glm::radians(-90.0f * dirVal), glm::vec3(0.0f, 1.0f, 0.0f));
        rootGargoyleNode->setNodeTransformation(mat);
    }

    // Add model
    std::shared_ptr<SVE::Entity> meshEntity = std::make_shared<SVE::MeshEntity>("gargoyle");
    meshEntity->setMaterial(gargoyle.type == GargoyleType::Fire ? "FireGargoyle" : "FrostGargoyle");
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

    const std::string particleSystemName = gargoyle.type == GargoyleType::Fire ? "FireLineParticle" : "FrostLineParticle";
    std::shared_ptr<SVE::ParticleSystemEntity> particleSystem = std::make_shared<SVE::ParticleSystemEntity>(particleSystemName);
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
    if (gargoyle.type == GargoyleType::Fire)
    {
        lightSettings.diffuseStrength = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
        lightSettings.specularStrength = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        lightSettings.ambientStrength = glm::vec4(0.2f, 0.15f, 0.15f, 1.0f);
    }
    else
    {
        lightSettings.diffuseStrength = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);
        lightSettings.specularStrength = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        lightSettings.ambientStrength = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);
    }

    lightSettings.shininess = 16;
    lightSettings.constAtten = 1.0f * 0.8f;
    lightSettings.linearAtten = 0.35f * 0.15f;
    lightSettings.quadAtten = 0.44f * 0.15f;
    auto lightManager = engine->getSceneManager()->getLightManager();
    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings, lightManager->getLightCount());
    lightManager->setLight(lightNode, lightManager->getLightCount());
    rootGargoyleNode->attachSceneNode(lightNode);
    gargoyle.startPoint = rootPos;
    gargoyle.lightNode = lightNode;

    _gargoyles.push_back(std::move(gargoyle));
}

void GameMap::finalizeGargoyle(Gargoyle &gargoyle)
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

        if (x >= _width || x < 0 || y >= _height || y < 0)
            stop = true;
        else
        {
            switch (_mapData[x][y].cellType)
            {
                case CellType::Wall:
                case CellType::InvisibleWallWithFloor:
                case CellType::InvisibleWallEmpty:
                    stop = true;
                    break;
            }
        }
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

void GameMap::update(float time)
{
    if (time <= 0.0)
        return;

    for (auto& teleport : _teleports)
    {
        updateTeleport(time, teleport);
    }

    for (auto& coin : _coins)
    {
        updateCoin(time, coin);
    }

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

void GameMap::initTeleportMesh()
{
    auto meshSettings = constructPlane("TeleportBase",
                                       glm::vec3(0, 0, 0), 0.9f, 0.9f, glm::vec3(0.0f, 1.0f, 0.0f));
    meshSettings.materialName = "TeleportBaseMaterial";
    auto teleportBaseMesh = std::make_shared<SVE::Mesh>(meshSettings);
    SVE::Engine::getInstance()->getMeshManager()->registerMesh(teleportBaseMesh);
}

void GameMap::createTeleport(int row, int column, char mapType)
{
    auto* engine = SVE::Engine::getInstance();

    Teleport teleport {};
    glm::vec3 color = {};
    switch (mapType)
    {
        case 'r':
            teleport.type = TeleportType::Red;
            color = {1.0f, 0.0, 0.0};
            break;
        case 'g':
            teleport.type = TeleportType::Green;
            color = {0.0f, 1.0, 0.0};
            break;
        case 'v':
            teleport.type = TeleportType::Purple;
            color = {1.0f, 0.0, 1.0};
            break;
        case 'y':
            teleport.type = TeleportType::Blue;
            color = {0.0f, 1.0, 1.0};
            break;
        default:
            assert(!"Unknown teleport type");
            return;
    }

    auto adjustColor = [](float comp, float step = 0.4f)
    {
        return comp < 0.01f ? step : comp;
    };

    auto adjustColor3 = [&adjustColor](glm::vec3 color, float num)
    {
        return glm::vec3(adjustColor(color.r, num), adjustColor(color.g, num), adjustColor(color.b, num));
    };

    auto position = getWorldPos(row, column, 0.2f);

    auto teleportNode = engine->getSceneManager()->createSceneNode();
    teleportNode->setNodeTransformation(glm::translate(glm::mat4(1), position));
    engine->getSceneManager()->getRootNode()->attachSceneNode(teleportNode);
    std::shared_ptr<SVE::ParticleSystemEntity> teleportPS = std::make_shared<SVE::ParticleSystemEntity>("TeleportStars");
    teleportPS->getMaterialInfo()->diffuse = glm::vec4(color, 1.0f);
    teleportNode->attachEntity(teleportPS);

    auto teleportCircleNode = engine->getSceneManager()->createSceneNode();
    auto teleportGlowNode = engine->getSceneManager()->createSceneNode();
    auto teleportLightNode = engine->getSceneManager()->createSceneNode();
    auto teleportPlatformNode = engine->getSceneManager()->createSceneNode();
    teleportNode->attachSceneNode(teleportCircleNode);
    teleportNode->attachSceneNode(teleportGlowNode);
    teleportNode->attachSceneNode(teleportLightNode);
    teleportNode->attachSceneNode(teleportPlatformNode);
    {
        teleportPlatformNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-1.0, -0.2, 1.0)));
        std::shared_ptr<SVE::MeshEntity> teleportPlatformEntity = std::make_shared<SVE::MeshEntity>("teleport");
        teleportPlatformEntity->setMaterial("TeleportPlatformMaterial");
        teleportPlatformEntity->getMaterialInfo()->diffuse = { adjustColor3(color, 0.4f), 1.0f };
        teleportPlatformNode->attachEntity(teleportPlatformEntity);

        std::shared_ptr<SVE::MeshEntity> teleportBaseEntity = std::make_shared<SVE::MeshEntity>("TeleportBase");
        teleportBaseEntity->setRenderLast();
        teleportBaseEntity->setCastShadows(false);
        teleportBaseEntity->getMaterialInfo()->diffuse = { adjustColor3(color, 0.6f), 2.0f };
        teleportCircleNode->attachEntity(teleportBaseEntity);

        std::shared_ptr<SVE::MeshEntity> teleportCircleEntity = std::make_shared<SVE::MeshEntity>("cylinder");
        teleportCircleEntity->setMaterial("TeleportCircleMaterial");
        teleportCircleEntity->setRenderLast();
        teleportCircleEntity->setCastShadows(false);
        teleportCircleEntity->getMaterialInfo()->diffuse = { adjustColor3(color, 0.6f), 2.0f };
        teleportGlowNode->attachEntity(teleportCircleEntity);

        // Add light effect
        teleportLightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 1.5, 0)));
        SVE::LightSettings lightSettings {};
        lightSettings.lightType = SVE::LightType::PointLight;
        lightSettings.castShadows = false;
        lightSettings.diffuseStrength = glm::vec4(color, 1.0f);
        lightSettings.specularStrength = glm::vec4(color * 0.5f, 1.0f);
        lightSettings.ambientStrength = { color * 0.2f, 1.0f };
        lightSettings.shininess = 16;
        lightSettings.constAtten = 1.0f * 1.8f;
        lightSettings.linearAtten = 0.35f * 0.25f;
        lightSettings.quadAtten = 0.44f * 0.25f;
        auto lightManager = engine->getSceneManager()->getLightManager();
        auto lightNode = std::make_shared<SVE::LightNode>(lightSettings, lightManager->getLightCount());
        lightManager->setLight(lightNode, lightManager->getLightCount());
        teleportLightNode->attachSceneNode(lightNode);
    }

    teleport.circleNode = std::move(teleportCircleNode);
    teleport.glowNode = std::move(teleportGlowNode);

    teleport.rootNode = std::move(teleportNode);

    _teleports.push_back(teleport);
    auto& currentTeleport = _teleports.back();
    for (auto& otherTeleport : _teleports)
    {
        if (&currentTeleport != &otherTeleport &&
            currentTeleport.type == otherTeleport.type)
        {
            teleport.secondEnd = &otherTeleport;
            otherTeleport.secondEnd = &teleport;
        }
    }
}

void GameMap::updateTeleport(float time, Teleport &teleport)
{
    float currentTime = SVE::Engine::getInstance()->getTime();
    auto updateNode = [](std::shared_ptr<SVE::SceneNode>& node, float time)
    {
        auto nodeTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        node->setNodeTransformation(nodeTransform);
    };

    updateNode(teleport.circleNode, currentTime * 2);
    updateNode(teleport.glowNode, currentTime * 5);
}

Coin* GameMap::createCoin(int row, int column)
{
    auto* engine = SVE::Engine::getInstance();

    Coin coin {};

    auto position = getWorldPos(row, column, 1.0f);

    auto coinNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    transform = glm::rotate(transform, glm::radians((float)(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    coinNode->setNodeTransformation(transform);
    engine->getSceneManager()->getRootNode()->attachSceneNode(coinNode);
    auto coinMesh = std::make_shared<SVE::MeshEntity>("coin");
    coinMesh->setMaterial("CoinMaterial");
    coinNode->attachEntity(coinMesh);

    coin.rootNode = std::move(coinNode);

    _coins.push_back(std::move(coin));

    return &_coins.back();
}

void GameMap::updateCoin(float time, Coin &coin)
{
    auto transform = coin.rootNode->getNodeTransformation();
    transform = glm::rotate(transform, time, glm::vec3(0.0f, 0.0f, 1.0f));
    coin.rootNode->setNodeTransformation(transform);
}

} // namespace Chewman