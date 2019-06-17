// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "GameMap.h"
#include "SVE/Engine.h"
#include "SVE/MeshManager.h"
#include "SVE/SceneManager.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{
constexpr float CellSize = 5.0f;

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

    _mapData.resize(_height);
    char ch;
    char line[100];
    for (auto row = 0; row < _height; ++row)
    {
        fin.getline(line, 100);
        _mapData[row].resize(_width);
        for (auto column = 0; column < _width; ++column)
        {
            fin >> ch;
            switch (ch)
            {
                case 'W':
                    _mapData[row][column] = CellType::Wall;
                    break;
                case 'L':
                    _mapData[row][column] = CellType::Liquid;
                    break;
                case '0':
                default:
                    _mapData[row][column] = CellType::Floor;
                    break;
            }
        }
    }

    InitMeshes();
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
    meshSettingsB.materialName = "FloorNormals";
    auto meshSettingsV = _meshGenerator.CombineMeshes("MapV", vertical);
    meshSettingsV.materialName = "WallParallax";

    auto* engine = SVE::Engine::getInstance();

    _mapMesh[0] = std::make_shared<SVE::Mesh>(meshSettingsT);
    engine->getMeshManager()->registerMesh(_mapMesh[0]);
    _mapMesh[1] = std::make_shared<SVE::Mesh>(meshSettingsB);
    engine->getMeshManager()->registerMesh(_mapMesh[1]);
    _mapMesh[2] = std::make_shared<SVE::Mesh>(meshSettingsV);
    engine->getMeshManager()->registerMesh(_mapMesh[2]);

    _mapNode = engine->getSceneManager()->createSceneNode();
    engine->getSceneManager()->getRootNode()->attachSceneNode(_mapNode);

    _mapEntity[0] = std::make_shared<SVE::MeshEntity>("MapT");
    _mapEntity[1] = std::make_shared<SVE::MeshEntity>("MapB");
    _mapEntity[2] = std::make_shared<SVE::MeshEntity>("MapV");

    //_mapNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(10, 5, 0)));
    _mapNode->attachEntity(_mapEntity[0]);
    _mapNode->attachEntity(_mapEntity[1]);
    _mapNode->attachEntity(_mapEntity[2]);
}

} // namespace Chewman