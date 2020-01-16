// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameMapLoader.h"
#include "GameUtils.h"

#include <SVE/Engine.h>
#include <SVE/MeshManager.h>
#include <SVE/SceneManager.h>
#include <SVE/ResourceManager.h>
#include <SVE/MaterialManager.h>

#include "Bomb.h"
#include "Game/Game.h"
#include "Game/Level/CustomEntity.h"
#include "Game/Level/Enemies/Nun.h"
#include "Game/Level/Enemies/Angel.h"
#include "Game/Level/Enemies/ChewmanEnemy.h"
#include "Game/Level/Enemies/Witch.h"
#include "Game/Level/Enemies/Knight.h"

#include <sstream>
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

    float s = sqrtf( (1+cosTheta)*2 );
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
                                       glm::vec3(0, 0, 0), 200.0f, 200.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    meshSettings.materialName = "SmokeMaterial";
    auto smokeMeshTop = std::make_shared<SVE::Mesh>(meshSettings);
    SVE::Engine::getInstance()->getMeshManager()->registerMesh(smokeMeshTop);

    /*meshSettings = constructPlane("SmokeFloor",
            glm::vec3(0, 0, 100 - CellSize * 0.5), 100.0f, 100.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    meshSettings.materialName = "SmokeMaterial";
    auto smokeMeshBottom = std::make_shared<SVE::Mesh>(meshSettings);
    SVE::Engine::getInstance()->getMeshManager()->registerMesh(smokeMeshBottom);*/
}

void initLevelBlocks(BlockMeshGenerator& meshGenerator)
{
    auto wallTop = meshGenerator.GenerateWall(glm::vec3(0,0,0), Top);
    auto wallVert = meshGenerator.GenerateWall(glm::vec3(0,0,0), Vertical);
    //auto lavaTop = meshGenerator.GenerateLiquid(glm::vec3(0,0,0), Top);

    auto meshSettingsWallT = meshGenerator.CombineMeshes("WallTop", wallTop);
    auto meshSettingsWallV = meshGenerator.CombineMeshes("WallVert", wallVert);

    auto wallMeshT = std::make_shared<SVE::Mesh>(meshSettingsWallT);
    auto wallMeshV = std::make_shared<SVE::Mesh>(meshSettingsWallV);

    auto* engine = SVE::Engine::getInstance();
    engine->getMeshManager()->registerMesh(wallMeshT);
    engine->getMeshManager()->registerMesh(wallMeshV);
}


void buildFloorMesh(const GameMap& level, BlockMeshGenerator& meshGenerator, std::string suffix)
{
    std::vector<Submesh> top;
    std::vector<Submesh> vertical;
    for (auto x = 0; x < level.height; ++x)
    {
        for (auto y = 0; y < level.width; ++y)
        {
            std::vector<Submesh> cellT;
            std::vector<Submesh> cellV;
            glm::vec3 position(y * CellSize, 0, -x * CellSize);
            switch (level.mapData[x][y].cellType)
            {
                case CellType::Wall:
                case CellType::Floor:
                case CellType::InvisibleWallWithFloor:
                    cellT = meshGenerator.GenerateFloor(position, Bottom);
                    cellV = meshGenerator.GenerateFloor(position, Vertical);
                    break;
            }
            top.insert(top.end(), cellT.begin(), cellT.end());
            vertical.insert(vertical.end(), cellV.begin(), cellV.end());
        }
    }

    auto liquid = meshGenerator.GenerateLiquidBorder(level.width, level.height);
    vertical.insert(vertical.end(), liquid.begin(), liquid.end());

    auto styleStr = std::to_string(level.style);
    auto meshSettingsB = meshGenerator.CombineMeshes("AllFloorTop" + suffix, top);
    meshSettingsB.materialName = "FloorNormals" + styleStr;
    auto meshSettingsV = meshGenerator.CombineMeshes("AllFloorVert" + suffix, vertical);
    meshSettingsV.materialName = "WallNormals" + styleStr;

    auto* engine = SVE::Engine::getInstance();
    engine->getMeshManager()->registerMesh(std::make_shared<SVE::Mesh>(meshSettingsB));
    engine->getMeshManager()->registerMesh(std::make_shared<SVE::Mesh>(meshSettingsV));
}

} // anon namespace

GameMapLoader::GameMapLoader()
    : _meshGenerator(CellSize)
{
    initTeleportMesh();
    initSmokeMesh();
    initLevelBlocks(_meshGenerator);
}

std::shared_ptr<GameMap> GameMapLoader::loadMap(const std::string& filename, const std::string& suffix)
{
    auto gameMap = std::make_shared<GameMap>();

    if (_callback)
        _callback(0);

    std::stringstream fin(SVE::Engine::getInstance()->getResourceManager()->loadFileContent(filename));
    fin >> gameMap->width >> gameMap->height;

    fin >> gameMap->timeFor3Stars >> gameMap->timeFor2Stars;

    fin >> gameMap->style >> gameMap->waterStyle;
    uint16_t light;
    fin >> light >> gameMap->treasureType;
    gameMap->isNight = light == 2;

    auto currentLevel = Game::getInstance()->getProgressManager().getCurrentLevel();
    if (currentLevel > 0 && Game::getInstance()->getGameSettingsManager().getSettings().switchLight[currentLevel - 1])
        gameMap->isNight = !gameMap->isNight;

    gameMap->mapNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    gameMap->upperLevelMeshNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    gameMap->mapNode->attachSceneNode(gameMap->upperLevelMeshNode);
    gameMap->unusedEntitiesNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();

    gameMap->mapData.resize(gameMap->height);
    char ch;
    char line[100] = {};

    fin.getline(line, 90);
    gameMap->name = line;
    gameMap->name.erase(std::find_if(gameMap->name.rbegin(), gameMap->name.rend(), [](char ch) {
        return !std::isspace(ch);
    }).base(), gameMap->name.end());

    auto& tutorialData = Game::getInstance()->getTutorialData();
    tutorialData.clear();
    if (gameMap->name.find("Tutorial") != std::string::npos)
    {
        for (auto i = 0; i < 4; ++i)
        {
            fin.getline(line, 90);
            gameMap->tutorialText.emplace_back(line);
            tutorialData.emplace_back(line);
        }
        gameMap->hasTutorial = true;
    } else {
        gameMap->hasTutorial = false;
    }

    // Static object help
    int nextIsRotation = 0;
    char staticObjectType = 0;

    std::map<std::pair<size_t, size_t>, CellType> definedCellType;
    auto addCellTypes = [&definedCellType](CellType cellType, int row, int column, std::pair<size_t, size_t> size)
    {
        for (auto x = row; x > row - (int)size.first; --x)
            for (auto y = column; y < column + (int)size.second; ++y)
                definedCellType[{x, y}] = cellType;
    };

    gameMap->coins.reserve(gameMap->width * gameMap->height);
    gameMap->powerUps.reserve(gameMap->width * gameMap->height);
    gameMap->teleports.reserve(gameMap->width * gameMap->height);
    for (auto row = 0u; row < gameMap->height; ++row)
    {
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
                    addCellTypes(StaticObject::getCellType(staticObjectType), curRow, column - 1, size);

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
                    gameMap->mapData[curRow][column].teleport = &gameMap->teleports.back();
                    break;
                case 'E':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->enemies.push_back(std::make_unique<Nun>(gameMap.get(), glm::ivec2(curRow, column)));
                    break;
                case 'Q':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->enemies.push_back((std::make_unique<Angel>(gameMap.get(), glm::ivec2(curRow, column))));
                    break;
                case 'R':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->enemies.push_back((std::make_unique<ChewmanEnemy>(gameMap.get(), glm::ivec2(curRow, column))));
                    break;
                case 'M':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->enemies.push_back(std::make_unique<Witch>(gameMap.get(), glm::ivec2(curRow, column)));
                    break;
                case 'K':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->enemies.push_back(std::make_unique<Knight>(gameMap.get(), glm::ivec2(curRow, column)));
                    break;
                case 'J':
                case 'D':
                case 'V':
                case 'Z':
                case 'Y':
                    nextIsRotation = (ch == 'D') ? 2 : 1;
                    staticObjectType = ch;
                    gameMap->mapData[curRow][column].cellType = StaticObject::getCellType(ch);
                    break;
                case 'P':
                case 'F':
                case 'A':
                case 'X':
                case 'H':
                case 'T':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->powerUps.emplace_back(std::make_unique<PowerUp>(gameMap.get(), glm::ivec2(curRow, column), ch));
                    gameMap->mapData[curRow][column].powerUp = gameMap->powerUps.back().get();
                    break;
                case 'B':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->powerUps.emplace_back(std::make_unique<Bomb>(gameMap.get(), glm::ivec2(curRow, column)));
                    gameMap->mapData[curRow][column].powerUp = gameMap->powerUps.back().get();
                    break;
                case 'S':
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    gameMap->player = std::make_shared<Player>(gameMap.get(), glm::ivec2(curRow, column));
                    break;
                default:
                    gameMap->mapData[curRow][column].cellType = CellType::Floor;
                    break;
            }
        }
        fin.getline(line, 100);
    }
    gameMap->activeCoins = gameMap->coins.size();
    gameMap->totalCoins = gameMap->coins.size();

    if (_callback)
        _callback(0.1);

    //fin.getline(line, 100);
    initMeshes(*gameMap, suffix);
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

    for (auto& enemy : gameMap->enemies)
        enemy->init();

    gameMap->eatEffectManager = std::make_unique<EatEffectManager>(gameMap.get());

    createSmoke(*gameMap);
    createLava(*gameMap, suffix);

    if (_callback) _callback(1.0);
    _callback = nullptr;

    return gameMap;
}

void GameMapLoader::initMeshes(GameMap& level, const std::string& suffix)
{
    buildFloorMesh(level, _meshGenerator, suffix);
    if (_callback)
        _callback(0.15);

    auto skinId = std::to_string(level.style);
    createLevelMaterial(skinId);

    for (auto x = 0; x < level.height; ++x)
    {
        for (auto y = 0; y < level.width; ++y)
        {
            glm::vec3 position(y * CellSize, 0, -x * CellSize);
            switch (level.mapData[x][y].cellType)
            {
                case CellType::Wall:
                {
                    auto node = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
                    level.upperLevelMeshNode->attachSceneNode(node);
                    node->setNodeTransformation(glm::translate(glm::mat4(1), position));

                    auto wallTop = std::make_shared<SVE::MeshEntity>("WallTop");
                    wallTop->setMaterial("CeilingNormals" + skinId);
                    node->attachEntity(std::move(wallTop));
                    auto wallVert = std::make_shared<SVE::MeshEntity>("WallVert");
                    wallVert->setMaterial("WallNormalsInstanced" + skinId);
                    node->attachEntity(wallVert);

                    level.mapData[x][y].cellBlock = std::move(node);
                    break;
                }
                case CellType::Floor:
                case CellType::InvisibleWallWithFloor:
                case CellType::Liquid:
                    break;
            }
        }
    }

    level.floor[0] = std::make_shared<SVE::MeshEntity>("AllFloorTop" + suffix);
    level.floor[1] = std::make_shared<SVE::MeshEntity>("AllFloorVert" + suffix);
    auto floorNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    level.mapNode->attachSceneNode(floorNode);
    floorNode->attachEntity(level.floor[0]);
    floorNode->attachEntity(level.floor[1]);

    if (_callback) _callback(0.9);
}

void GameMapLoader::createGargoyle(GameMap& level, int row, int column, char mapType)
{
    auto* engine = SVE::Engine::getInstance();
    auto& settings = Game::getInstance()->getGraphicsManager().getSettings();

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
    if (Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::Simple)
    {
        meshEntity->getMaterialInfo()->ambient = {0.2, 0.2, 0.2, 1.0};
    }
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

    if (settings.particleEffects == ParticlesSettings::Full)
    {
        const std::string particleSystemName = gargoyle.type == GargoyleType::Fire ? "FireLineParticle" : "FrostLineParticle";
        std::shared_ptr<SVE::ParticleSystemEntity> particleSystem = std::make_shared<SVE::ParticleSystemEntity>(particleSystemName);
        {
            auto &emitter = particleSystem->getSettings().particleEmitter;
            emitter.minLife = 0;
            emitter.maxLife = 0;
        }
        particlesNode->attachEntity(particleSystem);
        gargoyle.particleSystem = particleSystem;
        gargoyle.isFireline = false;
    } else {
        FireLineInfo info {};
        info.percent = 0.0f;
        info.alpha = 0.0f;
        info.direction = glm::vec3(0,0,1.0);
        info.startPos = glm::vec3(0);
        const std::string materialName = gargoyle.type == GargoyleType::Fire ? "FireLineMaterial" : "FrostLineMaterial";
        gargoyle.fireLine = std::make_shared<FireLineEntity>(materialName, info);
        gargoyle.fireLine->setRenderLast(true);
        gargoyle.isFireline = true;
        particlesNode->attachEntity(gargoyle.fireLine);
    }

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
    if (Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::Simple)
        lightSettings.isSimple = true;

    if (Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::High)
    {
        auto lightNode = std::make_shared<SVE::LightNode>(lightSettings);
        rootGargoyleNode->attachSceneNode(lightNode);
        gargoyle.lightNode = lightNode;
    }
    gargoyle.startPoint = rootPos;

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

        if (x >= level.height || x < 0 || y >= level.width || y < 0)
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
    if (gargoyle.isFireline)
    {
        auto& info = gargoyle.fireLine->getInfo();
        info.maxLength = length * CellSize + CellSize * (gargoyle.type == GargoyleType::Fire ? 0.5f : 0.3f);
        info.maxParticles = info.maxLength * (gargoyle.type == GargoyleType::Fire ? 75.0f : 50.0f);
    } else {
        auto& emitter = gargoyle.particleSystem->getSettings().particleEmitter;
        emitter.minSpeed = CellSize * length / gargoyle.fireTime;
        emitter.maxSpeed = emitter.minSpeed;
        //gargoyle.particleSystem->getSettings().quota = (gargoyle.fireTime + gargoyle.restTime) * 200;

        auto &affector = gargoyle.particleSystem->getSettings().particleAffector;
        affector.minScaleSpeed = gargoyle.type == GargoyleType::Fire ? (1.5f / gargoyle.fireTime) : (1.0f / gargoyle.fireTime);
        affector.maxScaleSpeed = affector.minScaleSpeed;
    }

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
    teleport.position = {row, column};
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

    auto teleportParticlesNode = engine->getSceneManager()->createSceneNode();
    if (Game::getInstance()->getGraphicsManager().getSettings().particleEffects == ParticlesSettings::None)
    {
        MagicInfo info {};
        info.color = color;
        teleport.sparks = std::make_shared<MagicEntity>("MagicMeshParticleMaterial", info);
        teleportParticlesNode->attachEntity(teleport.sparks);
    } else {
        std::shared_ptr<SVE::ParticleSystemEntity> teleportPS = std::make_shared<SVE::ParticleSystemEntity>("TeleportStars");
        teleportPS->getMaterialInfo()->diffuse = glm::vec4(color, 1.0f);
        teleportNode->attachEntity(teleportPS);
    }

    auto teleportCircleNode = engine->getSceneManager()->createSceneNode();
    auto teleportGlowNode = engine->getSceneManager()->createSceneNode();
    auto teleportLightNode = engine->getSceneManager()->createSceneNode();
    auto teleportPlatformNode = engine->getSceneManager()->createSceneNode();

    teleportNode->attachSceneNode(teleportCircleNode);
    teleportNode->attachSceneNode(teleportGlowNode);
    teleportNode->attachSceneNode(teleportLightNode);
    teleportNode->attachSceneNode(teleportParticlesNode);
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
        if (Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::High)
        {
            auto lightNode = std::make_shared<SVE::LightNode>(lightSettings);
            teleportLightNode->attachSceneNode(lightNode);
        }
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
            currentTeleport.secondEnd = &otherTeleport;
            otherTeleport.secondEnd = &currentTeleport;
        }
    }
}

Coin* GameMapLoader::createCoin(GameMap& level, int row, int column)
{
    auto* engine = SVE::Engine::getInstance();

    Coin coin {};

    float y = level.treasureType == 1 ? 1.0f : 0.5f;
    auto position = getWorldPos(row, column, y);

    auto coinNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    transform = glm::rotate(transform, glm::radians((float)(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    coinNode->setNodeTransformation(transform);
    level.mapNode->attachSceneNode(coinNode);
    auto coinMesh = std::make_shared<SVE::MeshEntity>(level.treasureType == 1 ? "coin" : "gem");
    if (Game::getInstance()->getGraphicsManager().getSettings().effectSettings == EffectSettings::Low)
        coinMesh->setMaterial(level.treasureType == 1 ? "CoinSimpleMaterial" : "GemSimpleMaterial");
    else
        coinMesh->setMaterial(level.treasureType == 1 ? "CoinMaterial" : "GemMaterial");
    coinNode->attachEntity(coinMesh);

    coin.rootNode = std::move(coinNode);

    level.coins.push_back(std::move(coin));

    return &level.coins.back();
}

void GameMapLoader::createLevelMaterial(const std::string& id)
{
    std::string skinTexturesFolder = "resources/materials/textures/level/";
    auto* materialManager = SVE::Engine::getInstance()->getMaterialManager();

    const auto createMaterial = [&](bool isInstanced, const std::string& name, const std::string& diffuse, const std::string& normals)
    {
        std::string materialName = name + id;
        auto* material = materialManager->getMaterial(materialName, true);
        if (!material)
        {
            SVE::MaterialSettings materialSettings{};
            materialSettings.name = materialName;
            materialSettings.vertexShaderName = isInstanced ? "phongNormalShadowInstancedVertexShader" : "phongNormalShadowVertexShader";
            materialSettings.fragmentShaderName = "phongNormalShadowFragmentShader";
            materialSettings.ignoreShadow = false;
            if (isInstanced)
            {
                materialSettings.useInstancing = true;
                materialSettings.instanceMaxCount = 20000;
            }

            SVE::TextureInfo textureInfo1 {};
            textureInfo1.samplerName = "diffuseTex";
            textureInfo1.filename = skinTexturesFolder + diffuse + id + ".jpg";

            SVE::TextureInfo textureInfo2 {};
            textureInfo2.samplerName = "normalTex";
            textureInfo2.filename = skinTexturesFolder + normals + id + ".jpg";

            SVE::TextureInfo textureInfo3 {};
            textureInfo3.samplerName = "directShadowTex";
            textureInfo3.textureType = SVE::TextureType::ShadowMapDirect;

            materialSettings.textures.push_back(textureInfo1);
            materialSettings.textures.push_back(textureInfo2);
            materialSettings.textures.push_back(textureInfo3);

            materialManager->registerMaterial(std::make_shared<SVE::Material>(materialSettings));
        }
    };

    createMaterial(true, "CeilingNormals", "ceiling", "ceilingNormal");
    if (_callback) _callback(0.3);
    createMaterial(false, "FloorNormals", "floor", "floorNormals");
    if (_callback) _callback(0.45);
    createMaterial(false, "WallNormals", "wall", "wallNormals");
    if (_callback) _callback(0.6);
    createMaterial(true, "WallNormalsInstanced", "wall", "wallNormals");
    if (_callback) _callback(0.75);
}

void GameMapLoader::createLava(GameMap& level, const std::string& suffix) const
{
    auto* engine = SVE::Engine::getInstance();
    auto liquidMeshSettings = constructPlane(
            "LiquidMesh" + suffix,
            glm::vec3(0.0f),
            level.width * CellSize * 0.5, level.height * CellSize  * 0.5,
            glm::vec3(0.0f, 1.0f, 0.0f));
    liquidMeshSettings.materialName = "LavaMaterial";
    auto liquidMesh = std::make_shared<SVE::Mesh>(liquidMeshSettings);
    engine->getMeshManager()->registerMesh(liquidMesh);

    auto lavaNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    lavaNode->setNodeTransformation(
            glm::translate(glm::mat4(1), glm::vec3((level.width - 1) * CellSize * 0.5f, -0.5f, -((level.height - 1) * CellSize * 0.5f))));
    level.mapNode->attachSceneNode(lavaNode);
    std::shared_ptr<SVE::MeshEntity> lavaEntity = std::make_shared<SVE::MeshEntity>("LiquidMesh" + suffix);
    bool isLowSettings = Game::getInstance()->getGraphicsManager().getSettings().effectSettings == EffectSettings::Low;
    switch (level.waterStyle)
    {
        case 1:
            lavaEntity->setMaterial(isLowSettings ? "LavaSimpleMaterial" : "LavaMaterial");
            break;
        case 2:
            lavaEntity->setMaterial(isLowSettings ? "WaterSimpleMaterial" : "WaterMaterial");
            break;
        case 3:
            lavaEntity->setMaterial(isLowSettings ? "AcidSimpleMaterial" : "AcidMaterial");
            break;
        default:
            lavaEntity->setMaterial(isLowSettings ? "LavaSimpleMaterial" : "LavaMaterial");
    }
    lavaEntity->setCastShadows(false);
    lavaEntity->getMaterialInfo()->ambient = {0.5f, 0.5f, 0.5f, 1.0f};
    lavaEntity->getMaterialInfo()->diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    lavaNode->attachEntity(lavaEntity);

    level.lavaEntity = std::move(lavaEntity);
}

void GameMapLoader::createSmoke(GameMap& level) const
{
    auto smokeNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    smokeNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -0.52, 0)));
    level.mapNode->attachSceneNode(smokeNode);
    std::shared_ptr<SVE::MeshEntity> smokeEntity = std::make_shared<SVE::MeshEntity>("SmokeFloor");
    smokeEntity->setMaterial("SmokeMaterial");
    smokeEntity->setCustomData(level.width * CellSize);
    //smokeEntity->setRenderLast();
    smokeEntity->setCastShadows(false);
    smokeNode->attachEntity(smokeEntity);

    /*auto smokeNode2 = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    smokeNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -0.52, 0)));
    level.mapNode->attachSceneNode(smokeNode2);
    std::shared_ptr<SVE::MeshEntity> smokeEntity2 = std::make_shared<SVE::MeshEntity>("SmokeFloor");
    smokeEntity2->setMaterial("SmokeMaterial");
    //smokeEntity->setRenderLast();
    smokeEntity2->setCastShadows(false);
    smokeNode2->attachEntity(smokeEntity2);

    level.smokeNAEntity = std::move(smokeEntity2);*/
}

void GameMapLoader::setCallback(CallbackFunc callback)
{
    _callback = callback;
}

} // namespace Chewman