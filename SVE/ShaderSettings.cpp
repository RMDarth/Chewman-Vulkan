// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Libs.h"
#include "ShaderSettings.h"
#include "ParticleSystemSettings.h"
#include "VulkanException.h"

namespace SVE
{

const std::map<UniformType, size_t>& getUniformSizeMap()
{
    static const std::map<UniformType, size_t> uniformSizeMap {
            { UniformType::ModelMatrix, sizeof(glm::mat4) },
            { UniformType::ViewMatrix, sizeof(glm::mat4) },
            { UniformType::ProjectionMatrix, sizeof(glm::mat4) },
            { UniformType::InverseModelMatrix, sizeof(glm::mat4) },
            { UniformType::ModelViewProjectionMatrix, sizeof(glm::mat4) },
            { UniformType::ViewProjectionMatrix, sizeof(glm::mat4) },
            { UniformType::ViewProjectionMatrixList, sizeof(glm::mat4) },
            { UniformType::ViewProjectionMatrixSize, sizeof(glm::ivec4) }, // float[3] padding
            { UniformType::CameraPosition, sizeof(glm::vec4) },
            { UniformType::MaterialInfo, sizeof(MaterialInfo) },
            { UniformType::LightInfo, sizeof(LightInfo) },
            { UniformType::LightSpot, sizeof(SpotLight) },
            { UniformType::LightPoint, sizeof(PointLight) },
            { UniformType::LightPointSimple, sizeof(PointLight) },
            { UniformType::LightLine, sizeof(LineLight) },
            { UniformType::LightDirectional, sizeof(DirLight) },
            { UniformType::LightPointViewProjectionList, sizeof(glm::mat4) },
            { UniformType::LightDirectViewProjectionList, sizeof(glm::mat4) },
            { UniformType::LightDirectViewProjection, sizeof(glm::mat4) },
            { UniformType::BoneMatrices, sizeof(glm::mat4) },
            { UniformType::ClipPlane, sizeof(glm::vec4) },
            { UniformType::ParticleEmitter, sizeof(ParticleEmitter) },
            { UniformType::ParticleAffector, sizeof(ParticleAffector) },
            { UniformType::ParticleCount, sizeof(uint32_t) },
            { UniformType::SpritesheetSize, sizeof(glm::ivec2) },
            { UniformType::ImageSize, sizeof(glm::ivec4) },
            { UniformType::TextInfo, sizeof(UniformTextInfo) },
            { UniformType::GlyphInfoList, sizeof(GlyphInfo) },
            { UniformType::TextSymbolList, sizeof(TextSymbolInfo) },
            { UniformType::Time, sizeof(float) },
            { UniformType::DeltaTime, sizeof(float) },
    };
    return uniformSizeMap;
}

const std::map<BufferType, size_t>& getStorageBufferSizeMap()
{
    static const std::map<BufferType, size_t> bufferSizeMap {
            { BufferType::AtomicCounter, sizeof(uint32_t) },
            { BufferType::ModelMatrixList, 0 }
    };

    return bufferSizeMap;
}

std::vector<char> getStorageDataByType(const StorageData& data, BufferType type)
{
    const auto& sizeMap = getUniformSizeMap();
    switch (type)
    {
        case BufferType::AtomicCounter:
        {
            return std::vector<char>();
        }
        case BufferType::ModelMatrixList:
        {
            const char* byteData = reinterpret_cast<const char*>(data.modelList.data());
            return std::vector<char>(byteData, byteData + sizeof(glm::mat4) * data.modelList.size());
        }
    }

    throw VulkanException("Unsupported uniform type");
}

std::vector<char> getUniformDataByType(const UniformData& data, UniformType type)
{
    const auto& sizeMap = getUniformSizeMap();
    switch (type)
    {
        case UniformType::ModelMatrix:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.model);
            return std::vector<char>(byteData, byteData + sizeof(data.model));
        }
        case UniformType::ViewMatrix:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.view);
            return std::vector<char>(byteData, byteData + sizeof(data.view));
        }
        case UniformType::ProjectionMatrix:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.projection);
            return std::vector<char>(byteData, byteData + sizeof(data.projection));
        }
        case UniformType::InverseModelMatrix:
        {
            auto inverseModel = glm::inverse(data.model);
            const char* byteData = reinterpret_cast<const char*>(&inverseModel);
            return std::vector<char>(byteData, byteData + sizeof(inverseModel));
        }
        case UniformType::ModelViewProjectionMatrix:
        {
            glm::mat4 mvp = data.projection * data.view * data.model;
            const char* byteData = reinterpret_cast<const char*>(&mvp);
            return std::vector<char>(byteData, byteData + sizeof(mvp));
        }
        case UniformType::ViewProjectionMatrix:
        {
            glm::mat4 vp = data.projection * data.view;
            const char* byteData = reinterpret_cast<const char*>(&vp);
            return std::vector<char>(byteData, byteData + sizeof(vp));
        }
        case UniformType::ViewProjectionMatrixList:
        {
            const char* byteData = reinterpret_cast<const char*>(data.viewProjectionList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.viewProjectionList.size());
        }
        case UniformType::ViewProjectionMatrixSize:
        {
            uint32_t vpListSize[4] =
                    { static_cast<uint32_t>(data.viewProjectionList.size()) };
            const char* byteData = reinterpret_cast<const char*>(vpListSize);
            return std::vector<char>(byteData, byteData + sizeMap.at(type));
        }
        case UniformType::CameraPosition:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.cameraPos);
            return std::vector<char>(byteData, byteData + sizeof(data.cameraPos));
        }
        case UniformType::MaterialInfo:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.materialInfo);
            return std::vector<char>(byteData, byteData + sizeof(data.materialInfo));
        }
        case UniformType::LightInfo:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.lightInfo);
            return std::vector<char>(byteData, byteData + sizeof(data.lightInfo));
        }
        case UniformType::LightDirectional:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.dirLight);
            return std::vector<char>(byteData, byteData + sizeMap.at(type));
        }
        case UniformType::LightPoint:
        {
            const char* byteData = reinterpret_cast<const char*>(data.shadowPointLightList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.shadowPointLightList.size());
        }
        case UniformType::LightPointSimple:
        {
            const char* byteData = reinterpret_cast<const char*>(data.pointLightList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.pointLightList.size());
        }
        case UniformType::LightLine:
        {
            const char* byteData = reinterpret_cast<const char*>(data.lineLightList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.lineLightList.size());
        }
        case UniformType::LightSpot:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.spotLight);
            return std::vector<char>(byteData, byteData + sizeMap.at(type));
        }
        case UniformType::LightPointViewProjectionList:
        {
            const char* byteData = reinterpret_cast<const char*>(data.lightPointViewProjectionList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.lightPointViewProjectionList.size());
        }
        case UniformType::LightDirectViewProjectionList:
        {
            const char* byteData = reinterpret_cast<const char*>(data.lightDirectViewProjectionList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.lightDirectViewProjectionList.size());
        }
        case UniformType::LightDirectViewProjection:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.lightDirectViewProjectionList.front());
            return std::vector<char>(byteData, byteData + sizeMap.at(type));
        }
        case UniformType::BoneMatrices:
        {
            const char* byteData = reinterpret_cast<const char*>(data.bones.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.bones.size());
        }
        case UniformType::ClipPlane:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.clipPlane);
            return std::vector<char>(byteData, byteData + sizeof(data.clipPlane));
        }
        case UniformType::ParticleEmitter:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.particleEmitter);
            return std::vector<char>(byteData, byteData + sizeof(data.particleEmitter));
        }
        case UniformType::ParticleAffector:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.particleAffector);
            return std::vector<char>(byteData, byteData + sizeof(data.particleAffector));
        }
        case UniformType::ParticleCount:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.particleCount);
            return std::vector<char>(byteData, byteData + sizeof(data.particleCount));
        }
        case UniformType::SpritesheetSize:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.spritesheetSize);
            return std::vector<char>(byteData, byteData + sizeof(data.spritesheetSize));
        }
        case UniformType::ImageSize:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.imageSize);
            return std::vector<char>(byteData, byteData + sizeof(data.imageSize));
        }
        case UniformType::TextInfo:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.textInfo);
            return std::vector<char>(byteData, byteData + sizeof(data.textInfo));
        }
        case UniformType::GlyphInfoList:
        {
            const char* byteData = reinterpret_cast<const char*>(data.glyphList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.glyphList.size());
        }
        case UniformType::TextSymbolList:
        {
            const char* byteData = reinterpret_cast<const char*>(data.textSymbolList.data());
            return std::vector<char>(byteData, byteData + sizeMap.at(type) * data.textSymbolList.size());
        }
        case UniformType::Time:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.time);
            return std::vector<char>(byteData, byteData + sizeof(data.time));
        }
        case UniformType::DeltaTime:
        {
            const char* byteData = reinterpret_cast<const char*>(&data.deltaTime);
            return std::vector<char>(byteData, byteData + sizeof(data.deltaTime));
        }
    }

    throw VulkanException("Unsupported uniform type");
}

void updateStorageDataByUniforms(const UniformData& data, StorageData& storageData, BufferType type)
{
    const auto& sizeMap = getUniformSizeMap();
    switch (type)
    {
        case BufferType::AtomicCounter:
        {
            return;
        }
        case BufferType::ModelMatrixList:
        {
            storageData.modelList.push_back(data.model);
            return;
        }
    }

    throw VulkanException("Unsupported SSBO type");
}

} // namespace SVE