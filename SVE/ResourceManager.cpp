// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ResourceManager.h"
#include "VulkanException.h"
#include "MaterialSettings.h"
#include "EngineSettings.h"
#include "ShaderSettings.h"
#include "MeshSettings.h"
#include "LightSettings.h"
#include "ParticleSystemSettings.h"

#include "Engine.h"
#include "ShaderManager.h"
#include "SceneManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "Libs.h"

#include <map>
#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>
#include <cppfs/FilePath.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <rapidjson/document.h>
#include <glm/gtx/quaternion.hpp>

#define setOptional(expr)                \
    try { expr; }                        \
    catch (const RapidJsonException&) { }

namespace SVE
{
namespace
{
namespace rj = rapidjson;

template<size_t vectorSize = 3, typename resultType = float, typename ObjectType>
glm::vec<vectorSize, resultType, glm::highp> loadVector(ObjectType &object, const std::string &name)
{
    auto vecArray = object[name.c_str()].GetArray();

    glm::vec<vectorSize, resultType, glm::highp> v;
    for (auto i = 0u; i < vectorSize; i++)
    {
        v[i] = vecArray[i].GetFloat();
    }

    return v;
}

EngineSettings loadEngine(const std::string& data)
{
    static const std::map<std::string, EngineSettings::PresentMode> presentModeMap{
            {"FIFO",          EngineSettings::PresentMode::FIFO},
            {"Mailbox",       EngineSettings::PresentMode::Mailbox},
            {"Immediate",     EngineSettings::PresentMode::Immediate},
            {"BestAvailable", EngineSettings::PresentMode::BestAvailable}
    };

    rj::Document document;
    document.Parse(data.c_str());

    EngineSettings engineSettings {};
    setOptional(engineSettings.useValidation = document["useValidation"].GetBool());
    setOptional(engineSettings.presentMode = presentModeMap.at(document["presentMode"].GetString()));
    setOptional(engineSettings.gpuIndex =
            document["gpuIndex"].IsString()
                ? (document["gpuIndex"].GetString() == std::string("best")
                        ? EngineSettings::BEST_GPU_AVAILABLE
                        : throw VulkanException("Incorrect gpu index"))
                : document["gpuIndex"].GetInt());
    setOptional(engineSettings.MSAALevel =
            document["MSAALevel"].IsString()
                ? (document["MSAALevel"].GetString() == std::string("best")
                       ? EngineSettings::BEST_MSAA_AVAILABLE
                       : throw VulkanException("Incorrect MSAA level"))
                : document["MSAALevel"].GetInt());
    setOptional(engineSettings.applicationName = document["applicationName"].GetString());
    setOptional(engineSettings.initShadows = document["initShadows"].GetBool());
    setOptional(engineSettings.initWater = document["initWater"].GetBool());
    setOptional(engineSettings.useScreenQuad = document["useScreenQuad"].GetBool());

    return engineSettings;
}

std::vector<UniformInfo> getUniformInfoList(rj::Document& document)
{
    static const std::map<std::string, UniformType> uniformMap{
            {"ModelMatrix",                     UniformType::ModelMatrix},
            {"ViewMatrix",                      UniformType::ViewMatrix},
            {"ProjectionMatrix",                UniformType::ProjectionMatrix},
            {"ModelViewProjectionMatrix",       UniformType::ModelViewProjectionMatrix},
            {"ViewProjectionMatrix",            UniformType::ViewProjectionMatrix},
            {"ViewProjectionMatrixList",        UniformType::ViewProjectionMatrixList},
            {"ViewProjectionMatrixSize",        UniformType::ViewProjectionMatrixSize},
            {"CameraPosition",                  UniformType::CameraPosition},
            {"MaterialInfo",                    UniformType::MaterialInfo},
            {"LightInfo",                       UniformType::LightInfo},
            {"LightDirectional",                UniformType::LightDirectional},
            {"LightPoint",                      UniformType::LightPoint},
            {"LightSpot",                       UniformType::LightSpot},
            {"LightPointViewProjectionList",    UniformType::LightPointViewProjectionList},
            {"LightDirectViewProjectionList",   UniformType::LightDirectViewProjectionList},
            {"BoneMatrices",                    UniformType::BoneMatrices},
            {"ClipPlane",                       UniformType::ClipPlane},
            {"ParticleEmitter",                 UniformType::ParticleEmitter},
            {"ParticleAffector",                UniformType::ParticleAffector},
            {"ParticleCount",                   UniformType::ParticleCount},
            {"SpritesheetSize",                 UniformType::SpritesheetSize},
            {"Time",                            UniformType::Time},
            {"DeltaTime",                       UniformType::DeltaTime},
    };

    std::vector<UniformInfo> uniformList;
    auto list = document["uniformList"].GetArray();
    for (auto& item : list)
    {
        UniformInfo uniformInfo {};
        uniformInfo.uniformType = uniformMap.at(item["uniformType"].GetString());
        setOptional(uniformInfo.uniformIndex = item["uniformIndex"].GetInt());
        uniformList.push_back(std::move(uniformInfo));
    }

    return uniformList;
}

std::vector<BufferType> getBufferTypeList(rj::Document& document)
{
    static const std::map<std::string, BufferType> bufferMap{
            {"AtomicCounter",     BufferType::AtomicCounter },
    };

    std::vector<BufferType> bufferList;
    auto list = document["bufferList"].GetArray();
    for (auto& item : list)
    {
        auto bufferType = bufferMap.at(item.GetString());
        bufferList.push_back(bufferType);
    }

    return bufferList;
}

VertexInfo getVertexInfo(rj::Document& document)
{
    static const std::map<std::string, VertexInfo::VertexDataType> vertexDataTypeMap{
            {"Position",    VertexInfo::VertexDataType::Position},
            {"Color",       VertexInfo::VertexDataType::Color},
            {"TexCoord",    VertexInfo::VertexDataType::TexCoord},
            {"Normal",      VertexInfo::VertexDataType::Normal},
            {"BoneWeights", VertexInfo::VertexDataType::BoneWeights},
            {"BoneIds",     VertexInfo::VertexDataType::BoneIds},
            {"Custom",      VertexInfo::VertexDataType::Custom},
    };

    VertexInfo info {};

    auto vertexInfo = document["vertexInfo"].GetObject();
    auto vertexDataFlags = vertexInfo["vertexDataFlags"].GetArray();

    info.vertexDataFlags = 0;
    for (auto& item : vertexDataFlags)
    {
        info.vertexDataFlags |= vertexDataTypeMap.at(item.GetString());
    }
    setOptional(info.positionSize = static_cast<uint8_t>(vertexInfo["positionSize"].GetUint()));
    setOptional(info.colorSize = static_cast<uint8_t>(vertexInfo["colorSize"].GetUint()));
    setOptional(info.customCount = static_cast<uint8_t>(vertexInfo["customCount"].GetUint()));
    setOptional(info.separateBinding = static_cast<uint8_t>(vertexInfo["separateBinding"].GetBool()));

    return info;
}

std::vector<std::string> getStringList(rj::Document& document, const std::string& listName)
{
    auto list = document[listName.c_str()].GetArray();

    std::vector<std::string> stringList;
    for (auto& item : list)
    {
        stringList.emplace_back(item.GetString());
    }

    return stringList;
}

ShaderSettings loadShader(const cppfs::FilePath& directory, const std::string& data)
{
    static const std::map<std::string, ShaderType> shaderTypeMap{
            {"VertexShader",   ShaderType::VertexShader},
            {"FragmentShader", ShaderType::FragmentShader},
            {"GeometryShader", ShaderType::GeometryShader},
            {"ComputeShader",  ShaderType::ComputeShader},
    };

    rj::Document document;
    document.Parse(data.c_str());

    ShaderSettings shaderSettings {};
    shaderSettings.name = document["name"].GetString();
    setOptional(shaderSettings.maxBonesSize = document["maxBonesSize"].GetUint());
    setOptional(shaderSettings.maxLightSize = document["maxLightSize"].GetUint());
    setOptional(shaderSettings.maxCascadeLightSize = document["maxCascadeLightSize"].GetUint());
    setOptional(shaderSettings.maxPointLightSize = document["maxPointLightSize"].GetUint());
    setOptional(shaderSettings.maxViewProjectionMatrices = document["maxViewProjectionMatrices"].GetUint());
    setOptional(shaderSettings.uniformList = getUniformInfoList(document));
    setOptional(shaderSettings.bufferList = getBufferTypeList(document));
    setOptional(shaderSettings.vertexInfo = getVertexInfo(document));
    setOptional(shaderSettings.samplerNamesList = getStringList(document, "samplerNamesList"));
    shaderSettings.filename = directory.resolve(document["filename"].GetString()).fullPath();
    shaderSettings.shaderType = shaderTypeMap.at(document["shaderType"].GetString());
    setOptional(shaderSettings.entryPoint = document["entryPoint"].GetString());

    return shaderSettings;
}

std::vector<TextureInfo> getTextureInfos(const cppfs::FilePath& directory, rj::Document& document)
{
    static const std::map<std::string, TextureType> textureTypeMap{
            {"ImageFile",       TextureType::ImageFile},
            {"ShadowMapDirect", TextureType::ShadowMapDirect},
            {"ShadowMapPoint",  TextureType::ShadowMapPoint},
            {"Reflection",      TextureType::Reflection},
            {"Refraction",      TextureType::Refraction},
            {"ScreenQuad",      TextureType::ScreenQuad},
    };

    static const std::map<std::string, TextureAddressMode> addressModeMap {
            { "Repeat",             TextureAddressMode::Repeat },
            { "MirroredRepeat",     TextureAddressMode::MirroredRepeat },
            { "ClampToEdge",        TextureAddressMode::ClampToEdge },
            { "ClampToBorder",      TextureAddressMode::ClampToBorder },
            { "MirrorClampToEdge",  TextureAddressMode::MirrorClampToEdge },
    };

    static const std::map<std::string, TextureBorderColor> borderColorMap {
            { "TransparentBlack",   TextureBorderColor::TransparentBlack },
            { "SolidBlack",         TextureBorderColor::SolidBlack },
            { "SolidWhite",         TextureBorderColor::SolidWhite },
    };

    auto list = document["textures"].GetArray();
    std::vector<TextureInfo> textureInfosList;

    for (auto& item : list)
    {
        TextureInfo textureInfo {};

        setOptional(textureInfo.textureType = textureTypeMap.at(item["textureType"].GetString()));
        setOptional(textureInfo.textureAddressMode = addressModeMap.at(item["textureAddressMode"].GetString()));
        setOptional(textureInfo.textureBorderColor = borderColorMap.at(item["textureBorderColor"].GetString()));
        setOptional(textureInfo.layers = item["layers"].GetUint());
        setOptional((textureInfo.spritesheetSize = loadVector<2,int>(item, "spritesheetSize")));

        if (textureInfo.textureType == TextureType::ImageFile)
        {
            textureInfo.filename = directory.resolve(item["filename"].GetString()).fullPath();
        }
        textureInfo.samplerName = item["samplerName"].GetString();

        textureInfosList.push_back(std::move(textureInfo));
    }

    return textureInfosList;
}

ParticleSystemSettings loadParticleSystem(const cppfs::FilePath& directory, const std::string& data)
{
    rj::Document document;
    document.Parse(data.c_str());

    ParticleSystemSettings particleSettings {};
    particleSettings.name = document["name"].GetString();
    particleSettings.materialName = document["materialName"].GetString();
    particleSettings.computeShaderName = document["computeShaderName"].GetString();
    particleSettings.quota = document["quota"].GetUint();
    particleSettings.sort = document["sort"].GetBool();

    ParticleEmitter emitter {};
    auto emitterObject = document["particleEmitter"].GetObject();

    glm::vec3 direction = loadVector(emitterObject, "direction");
    emitter.toDirection = glm::toMat4(glm::rotation(glm::vec3(0,0,1), direction));
    emitter.angle = emitterObject["angle"].GetFloat();
    emitter.originRadius = emitterObject["originRadius"].GetFloat();

    emitter.emissionRate = emitterObject["emissionRate"].GetFloat();
    emitter.minLife = emitterObject["minLife"].GetFloat();
    emitter.maxLife = emitterObject["maxLife"].GetFloat();
    emitter.minSpeed = emitterObject["minSpeed"].GetFloat();
    emitter.maxSpeed = emitterObject["maxSpeed"].GetFloat();
    emitter.minSize = emitterObject["minSize"].GetFloat();
    emitter.maxSize = emitterObject["maxSize"].GetFloat();
    emitter.minRotate = emitterObject["minRotate"].GetFloat();
    emitter.maxRotate = emitterObject["maxRotate"].GetFloat();
    emitter.colorRangeStart = loadVector<4>(emitterObject, "colorRangeStart");
    emitter.colorRangeEnd = loadVector<4>(emitterObject, "colorRangeEnd");

    ParticleAffector affector;
    auto affectorObject = document["particleAffector"].GetObject();
    affector.minAcceleration = affectorObject["minAcceleration"].GetFloat();
    affector.maxAcceleration = affectorObject["maxAcceleration"].GetFloat();
    affector.minRotateSpeed = affectorObject["minRotateSpeed"].GetFloat();
    affector.maxRotateSpeed = affectorObject["maxRotateSpeed"].GetFloat();
    affector.minScaleSpeed = affectorObject["minScaleSpeed"].GetFloat();
    affector.maxScaleSpeed = affectorObject["maxScaleSpeed"].GetFloat();
    affector.colorChanger = loadVector<4>(affectorObject, "colorChanger");

    particleSettings.particleEmitter = std::move(emitter);
    particleSettings.particleAffector = std::move(affector);

    return particleSettings;
}

MaterialSettings loadMaterial(const cppfs::FilePath& directory, const std::string& data)
{
    static const std::map<std::string, MaterialCullFace> cullFaceMap {
            { "BackFace",   MaterialCullFace::BackFace },
            { "FrontFace",  MaterialCullFace::FrontFace },
            { "None",       MaterialCullFace::None },
    };

    static const std::map<std::string, CommandsType> passTypeMap {
            { "MainPass",               CommandsType::MainPass },
            { "ScreenQuadPass",         CommandsType::ScreenQuadPass },
            { "RefractionPass",         CommandsType::RefractionPass },
            { "ReflectionPass",         CommandsType::ReflectionPass },
            { "ShadowPassDirectLight",  CommandsType::ShadowPassDirectLight },
            { "ShadowPassPointLights",  CommandsType::ShadowPassPointLights },
    };

    rj::Document document;
    document.Parse(data.c_str());

    MaterialSettings materialSettings {};
    materialSettings.name = document["name"].GetString();
    setOptional(materialSettings.cullFace = cullFaceMap.at(document["cullFace"].GetString()));
    setOptional(materialSettings.useDepthTest = document["useDepthTest"].GetBool());
    setOptional(materialSettings.useDepthBias = document["useDepthBias"].GetBool());
    setOptional(materialSettings.useMultisampling = document["useMultisampling"].GetBool());
    setOptional(materialSettings.useAlphaBlending = document["useAlphaBlending"].GetBool());
    setOptional(materialSettings.isCubemap = document["isCubemap"].GetBool());
    setOptional(materialSettings.passType = passTypeMap.at(document["passType"].GetString()));
    setOptional(materialSettings.fragmentShaderName = document["fragmentShaderName"].GetString());
    setOptional(materialSettings.geometryShaderName = document["geometryShaderName"].GetString());
    setOptional(materialSettings.vertexShaderName = document["vertexShaderName"].GetString());
    setOptional(materialSettings.textures = getTextureInfos(directory, document));

    return materialSettings;
}

MeshLoadSettings loadMesh(const cppfs::FilePath& directory, const std::string& data)
{
    rj::Document document;
    document.Parse(data.c_str());

    MeshLoadSettings meshLoadSettings {};

    meshLoadSettings.filename = directory.resolve(document["filename"].GetString()).fullPath();
    meshLoadSettings.name = document["name"].GetString();
    setOptional(meshLoadSettings.switchYZ = document["switchYZ"].GetBool());

    return meshLoadSettings;
}

LightSettings loadLight(const std::string& data)
{
    static const std::map<std::string, LightType> lightTypeMap{
            {"PointLight",  LightType::PointLight},
            {"RectLight",   LightType::RectLight},
            {"SpotLight",   LightType::SpotLight},
            {"SunLight",    LightType::SunLight},
    };

    rj::Document document;
    document.Parse(data.c_str());

    LightSettings lightSettings {};

    lightSettings.lightType =  lightTypeMap.at(document["lightType"].GetString());
    lightSettings.lightColor = loadVector(document, "lightColor");
    lightSettings.shininess = document["shininess"].GetFloat();
    lightSettings.ambientStrength = document["ambientStrength"].GetFloat();
    lightSettings.specularStrength = document["specularStrength"].GetFloat();
    lightSettings.diffuseStrength = document["diffuseStrength"].GetFloat();
    setOptional(lightSettings.castShadows = document["castShadows"].GetBool());

    return lightSettings;
}

} // anon namespace

ResourceManager::ResourceManager(std::vector<std::string> folderList)
    : _folderList(std::move(folderList))
{
    loadResources();
}

ResourceManager::ResourceManager()
{
}

void ResourceManager::loadResources()
{
    LoadData data {};
    for (const auto& folder : _folderList)
    {
        cppfs::FileHandle fh = cppfs::fs::open(folder);

        if (fh.isDirectory())
        {
            loadDirectory(folder, data);
        } else if (fh.isFile())
        {
            loadFile(folder, data);
        } else if (!fh.exists())
        {
            throw VulkanException("Folder doesn't exist");
        }
    }

    initializeResources(data);
}

void ResourceManager::initializeResources(LoadData& data)
{
    auto* engine = Engine::getInstance();
    for (auto& shaderSettings : data.shaderList)
    {
        std::shared_ptr<SVE::ShaderInfo> vertexShader = std::make_shared<SVE::ShaderInfo>(shaderSettings);
        engine->getShaderManager()->registerShader(vertexShader);
    }
    for (auto& lightSettings : data.lightList)
    {
        engine->getSceneManager()->createLight(lightSettings);
    }
    for (auto& materialSettings : data.materialsList)
    {
        std::shared_ptr<SVE::Material> material = std::make_shared<SVE::Material>(materialSettings);
        engine->getMaterialManager()->registerMaterial(material);
    }
    for (auto& meshLoadSettings : data.meshList)
    {
        std::shared_ptr<SVE::Mesh> mesh = std::make_shared<SVE::Mesh>(meshLoadSettings);
        engine->getMeshManager()->registerMesh(mesh);
    }
    for (auto& particleSystemSettings : data.particleSystemList)
    {
        engine->getSceneManager()->createParticleSystem(particleSystemSettings);
    }
}

void ResourceManager::loadFolder(const std::string& folder)
{
    _folderList.push_back(folder);

    LoadData loadData {};
    loadDirectory(folder, loadData);
    initializeResources(loadData);
}

ResourceManager::LoadData ResourceManager::getLoadDataFromFolder(const std::string& folder)
{
    LoadData data {};
    cppfs::FileHandle fh = cppfs::fs::open(folder);

    if (fh.isDirectory())
    {
        loadDirectory(folder, data);
    } else if (fh.isFile())
    {
        loadFile(folder, data);
    } else if (!fh.exists())
    {
        throw VulkanException("Folder doesn't exist");
    }

    return data;
}

const std::vector<std::string> ResourceManager::getFolderList() const
{
    return _folderList;
}

void ResourceManager::loadDirectory(const std::string& directory, LoadData& loadData)
{
    cppfs::FileHandle dir = cppfs::fs::open(directory);
    cppfs::FilePath fp(directory);
    for (cppfs::FileIterator it = dir.begin(); it != dir.end(); ++it)
    {
        loadFile(fp.resolve(*it).fullPath(), loadData);
    }
}

void ResourceManager::loadFile(const std::string& filename, LoadData& loadData)
{
    cppfs::FileHandle file = cppfs::fs::open(filename);
    if (file.isDirectory())
        return;

    cppfs::FilePath fp (filename);
    auto type = fp.extension().substr(1);

    static const std::map<std::string, ResourceType> resourceTypeMap {
        {"engine", ResourceType::Engine},
        {"shader", ResourceType::Shader},
        {"material", ResourceType::Material},
        {"mesh", ResourceType::Mesh},
        {"light", ResourceType::Light},
        {"particle", ResourceType::ParticleSystem}
    };

    if (resourceTypeMap.find(type) == resourceTypeMap.end())
    {
        // TODO: Add logging system
        std::cout << "Skipping unsupported file " << filename << std::endl;
    }

    std::string fileContent = file.readFile();
    auto directory = cppfs::FilePath(fp.directoryPath());

    try
    {
        if (resourceTypeMap.find(type) == resourceTypeMap.end())
            return;
        switch (resourceTypeMap.at(type))
        {
            case ResourceType::Engine:
                loadData.engine.emplace_back(loadEngine(fileContent));
                break;
            case ResourceType::Shader:
                loadData.shaderList.emplace_back(loadShader(directory, fileContent));
                break;
            case ResourceType::Material:
                loadData.materialsList.emplace_back(loadMaterial(directory, fileContent));
                break;
            case ResourceType::Mesh:
                loadData.meshList.emplace_back(loadMesh(directory, fileContent));
                break;
            case ResourceType::Light:
                loadData.lightList.emplace_back(loadLight(fileContent));
                break;
            case ResourceType::ParticleSystem:
                loadData.particleSystemList.emplace_back(loadParticleSystem(directory, fileContent));
                break;
        }
    } catch (const std::exception& ex)
    {
        throw VulkanException(std::string("Can't load resource file ") + filename + ": " + ex.what());
    }
}

} // namespace SVE