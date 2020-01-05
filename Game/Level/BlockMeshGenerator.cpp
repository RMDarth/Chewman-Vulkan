// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "BlockMeshGenerator.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <numeric>

namespace Chewman
{

namespace
{
glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
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

enum class VerticalPlaneType : uint8_t
{
    Left,
    Right,
    Top,
    Bottom
};

Submesh constructVerticalPlane(glm::vec3 center, float width, float height, VerticalPlaneType type, float texX = 1.0f, float texY = 1.0f, float deltaX = 0, float deltaY = 0)
{
    Submesh submesh {};

    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec3 scaleInfo;
    switch (type)
    {
        case VerticalPlaneType::Left:
        {
            submesh.points =
                    {
                            {0.0f,   1.0f, -1.0f},
                            {0.0f,   1.0f,  1.0f},
                            {0.0f,  -1.0f, -1.0f},
                            {0.0f,  -1.0f, -1.0f},
                            {0.0f,   1.0f,  1.0f},
                            {0.0f,  -1.0f,  1.0f},
                    };
            normal = glm::vec3(-1, 0, 0);
            tangent = glm::vec3(0, 0, 1);
            bitangent = glm::vec3(0, -1, 0);
            scaleInfo = glm::vec3(0, width / 2, height / 2);
            break;
        }
        case VerticalPlaneType::Right:
        {
            submesh.points =
                    {
                            {0.0f,   1.0f,  1.0f},
                            {0.0f,   1.0f, -1.0f},
                            {0.0f,  -1.0f,  1.0f},
                            {0.0f,  -1.0f,  1.0f},
                            {0.0f,   1.0f, -1.0f},
                            {0.0f,  -1.0f, -1.0f},
                    };

            normal = glm::vec3(1, 0, 0);
            tangent = glm::vec3(0, 0, -1);
            bitangent = glm::vec3(0, -1, 0);
            scaleInfo = glm::vec3(0, width / 2, height / 2);
            break;
        }
        case VerticalPlaneType::Top:
        {
            submesh.points =
                    {
                            { -1.0f, 1.0f, 0.0f},
                            { 1.0f,  1.0f, 0.0f},
                            {-1.0f,  -1.0f, 0.0f},
                            {-1.0f,  -1.0f, 0.0f},
                            { 1.0f,  1.0f, 0.0f},
                            { 1.0f, -1.0f, 0.0f},
                    };

            normal = glm::vec3(0, 0, 1);
            tangent = glm::vec3(1, 0, 0);
            bitangent = glm::vec3(0, -1, 0);
            scaleInfo = glm::vec3(height / 2, width / 2, 0.0f);
            break;
        }
        case VerticalPlaneType::Bottom:
        {
            submesh.points =
                    {
                            { 1.0f,  1.0f, 0.0f},
                            {-1.0f,  1.0f, 0.0f},
                            { 1.0f, -1.0f, 0.0f},
                            { 1.0f, -1.0f, 0.0f},
                            {-1.0f,  1.0f, 0.0f},
                            {-1.0f, -1.0f, 0.0f},
                    };

            normal = glm::vec3(0, 0, -1);
            tangent = glm::vec3(-1, 0, 0);
            bitangent = glm::vec3(0, -1, 0);
            scaleInfo = glm::vec3(height / 2, width / 2, 0.0f);
            break;
        }
    }

    submesh.texCoords =
            {
                    {0, 0},
                    {1, 0},
                    {0, 1},
                    {0, 1},
                    {1, 0},
                    {1, 1}
            };

    for (auto& texCoord : submesh.texCoords)
    {
        texCoord.x = texCoord.x < 0.5f ? deltaX : texX;
        texCoord.y = texCoord.y < 0.5f ? deltaY : texY;
    }

    submesh.normals = Vec3List(6, normal);
    submesh.colors = Vec3List(6, glm::vec3(1.0f, 1.0f, 1.0f));

    submesh.binormals = Vec3List(6, bitangent);
    submesh.tangents = Vec3List(6, tangent);

    for (auto& point : submesh.points)
    {
        point = scaleInfo * point;
        point += center;
    }

    return submesh;

}

Submesh constructPlane(glm::vec3 center, float width, float height, glm::vec3 normal, float texX = 1.0f, float texY = 1.0f, float deltaX = 0, float deltaY = 0)
{
    Submesh submesh {};
    submesh.points =
            {
                    {-1.0f,  0.0f,  1.0f},
                    {-1.0f,  0.0f, -1.0f},
                    {1.0f,   0.0f, -1.0f},
                    {1.0f,   0.0f, -1.0f},
                    {1.0f,   0.0f, 1.0f},
                    {-1.0f,  0.0f, 1.0f},
            };
    submesh.texCoords =
            {
                    {1, 1},
                    {0, 1},
                    {0, 0},
                    {0, 0},
                    {1, 0},
                    {1, 1}
            };
    submesh.normals = Vec3List(6, normal);
    submesh.colors = Vec3List(6, glm::vec3(1.0f, 1.0f, 1.0f));

    for (auto& texCoord : submesh.texCoords)
    {
        texCoord.x = texCoord.x < 0.5f ? deltaX : texX;
        texCoord.y = texCoord.y < 0.5f ? deltaY : texY;
    }

    auto mat = glm::toMat4(rotationBetweenVectors(glm::vec3(0.0, 1.0, 0.0), normal));

    glm::vec3 tangent = glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 bitangent = glm::vec3(1.0, 0.0, 0.0);

    submesh.binormals = Vec3List(6, bitangent);
    submesh.tangents = Vec3List(6, tangent);

    mat = mat * glm::scale(glm::mat4(1), glm::vec3(width / 2, 0, height / 2));
    for (auto& point : submesh.points)
    {
        point = glm::vec3(mat * glm::vec4(point, 1.0f));
        point += center;
    }

    return submesh;
}

} // anon namespace

BlockMeshGenerator::BlockMeshGenerator(float size)
    : _size(size)
{
}

std::vector<Submesh> BlockMeshGenerator::GenerateFloor(glm::vec3 position, ModelType type)
{
    std::vector<Submesh> floorMeshes;
    float height = _size;
    if (type == ModelType::Bottom)
    {
        floorMeshes.push_back(constructPlane(position, _size, _size, glm::vec3(0, 1, 0), 1.0, 1.0, 0.0, 0.0));
    }
    else if (type == ModelType::Vertical)
    {
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(_size / 2, -height / 2, 0),
                                                     height, _size, VerticalPlaneType::Right, 1.0f, 2.0,  0.0f,  0.0f));
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(-_size / 2, -height / 2, 0),
                                                     height, _size, VerticalPlaneType::Left, 1.0f, 2.0,  0.0f,  0.0f));
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(0, -height / 2, _size / 2),
                                                     height, _size, VerticalPlaneType::Top, 1.0f, 2.0,  0.0f,  0.0f));
        // This is small optimization - back wall is never seen
        //floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(0, -height / 2, -_size / 2),
        //                                             height, _size, VerticalPlaneType::Bottom, 1.0f, 2.0,  0.0f,  0.0f));
    }
    return floorMeshes;
}

std::vector<Submesh> BlockMeshGenerator::GenerateWall(glm::vec3 position, ModelType type)
{
    std::vector<Submesh> floorMeshes;
    float subheight = _size; // / 5
    float mainheight = _size / 2;
    float height = mainheight + subheight;
    if (type == ModelType::Top)
    {
        floorMeshes.push_back(constructPlane(position + glm::vec3(0, mainheight, 0), _size, _size, glm::vec3(0, 1, 0)));
    } else if (type == ModelType::Vertical) {
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(_size / 2, mainheight/2 - subheight / 2, 0),
                              height, _size, VerticalPlaneType::Right, 1.0f, 3.0,  0.0f,  0.0f));
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(-_size / 2, mainheight/2 - subheight / 2, 0),
                              height, _size, VerticalPlaneType::Left, 1.0f, 3.0,  0.0f,  0.0f));
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(0,          mainheight/2 - subheight / 2,  _size / 2),
                                                     height, _size, VerticalPlaneType::Top, 1.0f, 1.4,  0.0f,  0.0f));
        floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(0,          mainheight/2 - subheight / 2,  -_size / 2),
                                                     height, _size, VerticalPlaneType::Bottom, 1.0f, 1.4,  0.0f,  0.0f));
    }

    return floorMeshes;

}

std::vector<Submesh> BlockMeshGenerator::GenerateLiquid(glm::vec3 position, ModelType type, int x, int y, int xMax, int yMax)
{
    std::vector<Submesh> floorMeshes;
    float height = _size;
    float wallY = -height/2 - height/6;
    if (type == ModelType::Vertical)
    {
        float texY = 1.33333f;
        float deltaY = 2.0f - texY;
        if (y == 0)
            floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(_size / 2, wallY, 0),
                                                     height, _size, VerticalPlaneType::Right, 1.0f, texY,  0.0f,  -deltaY));

        if (y == yMax)
            floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(-_size / 2, wallY, 0),
                                                     height, _size, VerticalPlaneType::Left, 1.0f, texY,  0.0f,  -deltaY));

        if (x == 0)
            floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(0, wallY, _size / 2),
                                                     height, _size, VerticalPlaneType::Top, 1.0f, texY,  0.0f,  -deltaY));
        if (x == xMax)
            floorMeshes.push_back(constructVerticalPlane(position + glm::vec3(0, wallY, -_size / 2),
                                                     height, _size, VerticalPlaneType::Bottom, 1.0f, texY,  0.0f,  -deltaY));
    }

    return floorMeshes;
}

SVE::MeshSettings BlockMeshGenerator::CombineMeshes(std::string name, std::vector<Submesh> meshes)
{
    SVE::MeshSettings meshSettings {};

    uint32_t totalPoints = 0;
    for (const auto& submesh : meshes)
    {
        std::copy(submesh.points.begin(), submesh.points.end(), std::back_inserter(meshSettings.vertexPosData));
        std::copy(submesh.texCoords.begin(), submesh.texCoords.end(), std::back_inserter(meshSettings.vertexTexData));
        std::copy(submesh.normals.begin(), submesh.normals.end(), std::back_inserter(meshSettings.vertexNormalData));
        std::copy(submesh.binormals.begin(), submesh.binormals.end(), std::back_inserter(meshSettings.vertexBinormalData));
        std::copy(submesh.tangents.begin(), submesh.tangents.end(), std::back_inserter(meshSettings.vertexTangentData));
        std::copy(submesh.colors.begin(), submesh.colors.end(), std::back_inserter(meshSettings.vertexColorData));
        totalPoints += submesh.points.size();
    }

    meshSettings.indexData.resize(totalPoints);
    std::iota(meshSettings.indexData.begin(), meshSettings.indexData.end(), 0);
    meshSettings.boneNum = 0;
    meshSettings.name = std::move(name);

    return meshSettings;
}

} // namespace Chewman