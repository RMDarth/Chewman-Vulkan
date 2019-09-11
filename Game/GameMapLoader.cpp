// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameMapLoader.h"
#include "GameUtils.h"

#include <SVE/Engine.h>
#include <SVE/MeshManager.h>
#include <SVE/SceneManager.h>
#include <SVE/LightManager.h>

#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Chewman
{
namespace
{

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

void initTeleportMesh()
{
    auto meshSettings = constructPlane("TeleportBase",
                                       glm::vec3(0, 0, 0), 0.9f, 0.9f, glm::vec3(0.0f, 1.0f, 0.0f));
    meshSettings.materialName = "TeleportBaseMaterial";
    auto teleportBaseMesh = std::make_shared<SVE::Mesh>(meshSettings);
    SVE::Engine::getInstance()->getMeshManager()->registerMesh(teleportBaseMesh);
}

void initSmokeMesh()
{
    auto meshSettings = constructPlane("SmokeFloor",
                                       glm::vec3(0, 0, 0), 1000.0f, 1000.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    meshSettings.materialName = "SmokeMaterial";
    auto teleportBaseMesh = std::make_shared<SVE::Mesh>(meshSettings);
    SVE::Engine::getInstance()->getMeshManager()->registerMesh(teleportBaseMesh);
}

} // anon namespace

GameMapLoader::GameMapLoader()
    : _meshGenerator(CellSize)
{
    initTeleportMesh();
    initSmokeMesh();
}

std::shared_ptr<GameMap> GameMapLoader::loadMap(const std::string& filename)
{
    auto gameMap = std::make_shared<GameMap>();

    std::ifstream fin(filename);
    fin >> gameMap->width >> gameMap->height;

    uint8_t style, waterStyle;
    std::string levelName;
    fin >> style >> waterStyle >> levelName;

    gameMap->mapNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    SVE::Engine::getInstance()->getSceneManager()->getRootNode()->attachSceneNode(gameMap->mapNode);

    gameMap->mapData.resize(gameMap->height);
    char ch;
    char line[100];

    // Static object help
    int nextIsRotation = 0;
    char staticObjectType = 0;

    std::map<std::pair<size_t, size_t>, CellType> definedCellType;
    auto addCellTypes = [&definedCellType](CellType cellType, size_t row, size_t column, std::pair<size_t, size_t> size)
    {
        for (auto x = row; x > row - size.first; --x)
            for (auto y = column; y < column + size.second; ++y)
                definedCellType[{x, y}] = cellType;
    };

    for (auto row = 0u; row < gameMap->height; ++row)
    {
        fin.getline(line, 100);
        auto curRow = gameMap->height - row - 1;
        gameMap->mapData[curRow].resize(gameMap->width);
        for (auto column = 0u; column < gameMap->width; ++column)
        {
            fin >> ch;
            gameMap->mapData[curRow][column] = {};

            if (nextIsRotation)
            {
                nextIsRotation--;
                if (staticObjectType != 0)
                {
                    gameMap->staticObjects.emplace_back(gameMap.get(), glm::ivec2(curRow, column), staticObjectType, ch);
                    auto size = StaticObject::getSize(staticObjectType, ch);
                    addCellTypes(StaticObject::getCellType(staticObjectType), curRow, column-1, size);

                    staticObjectType = 0;
                }
            }

            auto definedCellIt = definedCellType.find({curRow, column});
            if (definedCellIt != definedCellType.end())
            {
                gameMap->mapData[curRow][column].cellType = definedCellIt->second;
                continue;
            }


            switch (ch)
            {
                case 'W':
                    gameMap->mapData[curRow][column].cellType = CellType::Wall;
                    break;
                case 'L':
                    gameMap->mapData[curRow][column].cellType = CellType::Liquid;
                    break;
                case '0':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    break;
                case 'C':
                    gameMap->mapData[curRow][column].coin = createCoin(*gameMap, curRow, column);
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    gameMap->mapData[curRow][column].cellType = CellType::InvisibleWallWithFloor;
                    createGargoyle(*gameMap, curRow, column, ch);
                    break;
                case 'r':
                case 'g':
                case 'v':
                case 'y':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    createTeleport(*gameMap, curRow, column, ch);
                    break;
                case 'E':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->nuns.emplace_back(gameMap.get(), glm::ivec2(curRow, column));
                    break;
                case 'J':
                case 'D':
                case 'V':
                case 'Z':
                case 'Y':
                    nextIsRotation = ch == 'D' ? 2 : 1;
                    staticObjectType = ch;
                    gameMap->mapData[curRow][column].cellType = StaticObject::getCellType(ch);
                    break;
                case 'P':
                case 'F':
                case 'A':
                case 'X':
                case 'B':
                case 'H':
                case 'T':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->powerUps.emplace_back(gameMap.get(), glm::ivec2(curRow, column), ch);
                    break;
                default:
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    break;
            }
        }
    }

    fin.getline(line, 100);
    initMeshes(*gameMap);
    for (auto& gargoyle : gameMap->gargoyles)
    {
        uint32_t startTime, finishTime;
        fin >> startTime >> finishTime;
        // Old trashman used weird timing, when 10 clocks are used when 15 passed
        startTime *= 1.5f;
        finishTime *= 1.5f;
        gargoyle.restTime = (float)startTime / 1000;
        gargoyle.fireTime = (float)(finishTime - startTime) / 1000;
        finalizeGargoyle(*gameMap, gargoyle);
    }
    createSmoke(*gameMap);

    fin.close();

    return gameMap;
}

void GameMapLoader::initMeshes(GameMap& level)
{
    std::vector<Submesh> top;
    std::vector<Submesh> bottom;
    std::vector<Submesh> vertical;
    for (auto x = 0; x < level.height; ++x)
    {
        for (auto y = 0; y < level.width; ++y)
        {
            std::vector<Submesh> cellT;
            std::vector<Submesh> cellB;
            std::vector<Submesh> cellV;
            glm::vec3 position(y * CellSize, 0, -x * CellSize);
            switch (level.mapData[x][y].cellType)
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
                    //cellT = _meshGenerator.GenerateLiquid(position, ModelType::Top);
                    //cellB = _meshGenerator.GenerateLiquid(position, ModelType::Bottom);
                    //cellV = _meshGenerator.GenerateLiquid(position, ModelType::Vertical);
                    cellV = _meshGenerator.GenerateLiquid(position, ModelType::Vertical, x, y, level.height - 1, level.width - 1);
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

    std::shared_ptr<SVE::Mesh> mapMesh[3];
    mapMesh[0] = std::make_unique<SVE::Mesh>(meshSettingsT);
    engine->getMeshManager()->registerMesh(mapMesh[0]);
    mapMesh[1] = std::make_shared<SVE::Mesh>(meshSettingsB);
    engine->getMeshManager()->registerMesh(mapMesh[1]);
    mapMesh[2] = std::make_shared<SVE::Mesh>(meshSettingsV);
    engine->getMeshManager()->registerMesh(mapMesh[2]);

    level.mapEntity[0] = std::make_shared<SVE::MeshEntity>("MapT");
    level.mapEntity[1] = std::make_shared<SVE::MeshEntity>("MapB");
    level.mapEntity[2] = std::make_shared<SVE::MeshEntity>("MapV");
    for (auto i = 0; i < 3; ++i)
        level.mapEntity[i]->setRenderToDepth(true);

    //level.mapNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(10, 5, 0)));
    level.mapNode->attachEntity(level.mapEntity[0]);
    level.mapNode->attachEntity(level.mapEntity[1]);
    level.mapNode->attachEntity(level.mapEntity[2]);
}

void GameMapLoader::createGargoyle(GameMap& level, int row, int column, char mapType)
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
    level.mapNode->attachSceneNode(rootGargoyleNode);

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

    level.gargoyles.push_back(std::move(gargoyle));
}

void GameMapLoader::finalizeGargoyle(GameMap& level, Gargoyle &gargoyle)
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

        if (x >= level.width || x < 0 || y >= level.height || y < 0)
            stop = true;
        else
        {
            switch (level.mapData[x][y].cellType)
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

    auto &affector = gargoyle.particleSystem->getSettings().particleAffector;
    affector.minScaleSpeed = gargoyle.type == GargoyleType::Fire ? (1.5f / gargoyle.fireTime) : (1.0f / gargoyle.fireTime);
    affector.maxScaleSpeed = affector.minScaleSpeed;

    x-=speedX;
    y-=speedY;
    gargoyle.finalPoint = glm::vec3(CellSize * y, CellSize / 4, -CellSize * x);
    auto distance = gargoyle.finalPoint - gargoyle.startPoint;
    gargoyle.direction = glm::normalize(distance);
    gargoyle.totalLength = glm::length(distance);
}

void GameMapLoader::createTeleport(GameMap& level, int row, int column, char mapType)
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
    level.mapNode->attachSceneNode(teleportNode);
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

    level.teleports.push_back(teleport);
    auto& currentTeleport = level.teleports.back();
    for (auto& otherTeleport : level.teleports)
    {
        if (&currentTeleport != &otherTeleport &&
            currentTeleport.type == otherTeleport.type)
        {
            teleport.secondEnd = &otherTeleport;
            otherTeleport.secondEnd = &teleport;
        }
    }
}

Coin* GameMapLoader::createCoin(GameMap& level, int row, int column)
{
    auto* engine = SVE::Engine::getInstance();

    Coin coin {};

    auto position = getWorldPos(row, column, 1.0f);

    auto coinNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    transform = glm::rotate(transform, glm::radians((float)(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    coinNode->setNodeTransformation(transform);
    level.mapNode->attachSceneNode(coinNode);
    auto coinMesh = std::make_shared<SVE::MeshEntity>("coin");
    coinMesh->setMaterial("CoinMaterial");
    coinNode->attachEntity(coinMesh);

    coin.rootNode = std::move(coinNode);

    level.coins.push_back(std::move(coin));

    return &level.coins.back();
}

void GameMapLoader::createSmoke(GameMap& level) const
{
    auto smokeNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    smokeNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -0.5, 0)));
    level.mapNode->attachSceneNode(smokeNode);
    std::shared_ptr<SVE::MeshEntity> smokeEntity = std::make_shared<SVE::MeshEntity>("SmokeFloor");
    smokeEntity->setMaterial("SmokeMaterial");
    //smokeEntity->setRenderLast();
    //smokeEntity->setCastShadows(false);
    smokeNode->attachEntity(smokeEntity);

    level.smokeEntity = std::move(smokeEntity);
}

} // namespace Chewman