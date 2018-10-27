// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Skybox.h"
#include "Engine.h"
#include "MeshSettings.h"
#include "MaterialManager.h"
#include "MaterialSettings.h"
#include "Mesh.h"
#include "Material.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "Utils.h"

namespace SVE
{
static const float SIZE = 1.0f;

static const std::vector<glm::vec3> skyboxVertices = {
        {-SIZE,  SIZE, -SIZE},
        {-SIZE, -SIZE, -SIZE},
        {SIZE, -SIZE, -SIZE},
        {SIZE, -SIZE, -SIZE},
        {SIZE,  SIZE, -SIZE},
        {-SIZE,  SIZE, -SIZE},

        {-SIZE, -SIZE,  SIZE},
        {-SIZE, -SIZE, -SIZE},
        {-SIZE,  SIZE, -SIZE},
        {-SIZE,  SIZE, -SIZE},
        {-SIZE,  SIZE,  SIZE},
        {-SIZE, -SIZE,  SIZE},

        {SIZE, -SIZE, -SIZE},
        {SIZE, -SIZE,  SIZE},
        {SIZE,  SIZE,  SIZE},
        {SIZE,  SIZE,  SIZE},
        {SIZE,  SIZE, -SIZE},
        {SIZE, -SIZE, -SIZE},

        {-SIZE, -SIZE,  SIZE},
        {-SIZE,  SIZE,  SIZE},
        {SIZE,  SIZE,  SIZE},
        {SIZE,  SIZE,  SIZE},
        {SIZE, -SIZE,  SIZE},
        {-SIZE, -SIZE,  SIZE},

        {-SIZE,  SIZE, -SIZE},
        {SIZE,  SIZE, -SIZE},
        {SIZE,  SIZE,  SIZE},
        {SIZE,  SIZE,  SIZE},
        {-SIZE,  SIZE,  SIZE},
        {-SIZE,  SIZE, -SIZE},

        {-SIZE, -SIZE, -SIZE},
        {-SIZE, -SIZE,  SIZE},
        {SIZE, -SIZE, -SIZE},
        {SIZE, -SIZE, -SIZE},
        {-SIZE, -SIZE,  SIZE},
        {SIZE, -SIZE,  SIZE}
};

Skybox::Skybox(const std::string& materialName)
    : _material(Engine::getInstance()->getMaterialManager()->getMaterial(materialName))
{
    MeshSettings meshSettings;
    meshSettings.name = "Skybox";
    meshSettings.boneNum = 0;
    meshSettings.animation = nullptr;
    meshSettings.vertexPosData = skyboxVertices;
    for (uint32_t i = 0; i < skyboxVertices.size(); i++)
    {
        meshSettings.indexData.push_back(i);
    }
    meshSettings.materialName = materialName;

    _mesh = std::make_shared<Mesh>(meshSettings);

    if (_material)
        setupMaterial();
}

Skybox::~Skybox() = default;

void Skybox::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    // TODO: Probably should not be rendered on shadow pass (and possibly refraction)
    uint32_t materialIndex;
    switch (Engine::getInstance()->getPassType())
    {
        case CommandsType::MainPass:
            materialIndex = _materialIndex;
            break;
        case CommandsType::ReflectionPass:
            materialIndex = _reflectionMaterialIndex;
            break;
        case CommandsType::RefractionPass:
            materialIndex = _refractionMaterialIndex;
            break;
        default:
            materialIndex = _materialIndex;
    }
    _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, materialIndex);
    _mesh->getVulkanMesh()->applyDrawingCommands(bufferIndex);
}

void Skybox::updateUniforms(UniformDataList uniformDataList, const UniformDataIndexMap& indexMap) const
{
    _material->getVulkanMaterial()->setUniformData(_materialIndex, *uniformDataList[indexMap.at(CommandsType::MainPass)]);

    if (Engine::getInstance()->isWaterEnabled())
    {
        _material->getVulkanMaterial()->setUniformData(_reflectionMaterialIndex, *uniformDataList[indexMap.at(CommandsType::ReflectionPass)]);
        _material->getVulkanMaterial()->setUniformData(_refractionMaterialIndex, *uniformDataList[indexMap.at(CommandsType::RefractionPass)]);
    }
}

void Skybox::setupMaterial()
{
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);

    if (Engine::getInstance()->isWaterEnabled())
    {
        _reflectionMaterialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this, 1);
        _refractionMaterialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this, 2);
    }
}


} // namespace SVE