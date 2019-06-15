// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <cstdint>
#include "SVE/MeshSettings.h"

namespace Chewman
{

using Vec3List = std::vector<glm::vec3>;

struct Submesh
{
    Vec3List points;
    std::vector<glm::vec2> texCoords;
    Vec3List normals;
    Vec3List colors;
};

enum ModelType
{
    Vertical,
    Top,
    Bottom
};

class BlockMeshGenerator
{
public:
    BlockMeshGenerator(float size);

    std::vector<Submesh> GenerateFloor(glm::vec3 position, ModelType type);
    std::vector<Submesh> GenerateWall(glm::vec3 position, ModelType type);
    std::vector<Submesh> GenerateLiquid(glm::vec3 position, ModelType type);

    SVE::MeshSettings CombineMeshes(std::string name, std::vector<Submesh> meshes);

private:
    float _size;
};

} // namespace Chewman