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
        _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
}

Skybox::~Skybox() = default;

void Skybox::applyDrawingCommands(uint32_t bufferIndex) const
{
    // TODO: Probably should not be rendered on shadow pass (and possibly refraction)
    _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, _materialIndex);
    _mesh->getVulkanMesh()->applyDrawingCommands(bufferIndex);
}

void Skybox::updateUniforms(UniformDataList uniformDataList) const
{
    _material->getVulkanMaterial()->setUniformData(_materialIndex, *uniformDataList[toInt(CommandsType::MainPass)]);
}


} // namespace SVE