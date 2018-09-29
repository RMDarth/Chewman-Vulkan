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

#include "Engine.h"
#include "ShaderManager.h"
#include "SceneManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"

#include <map>
#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>
#include <cppfs/FilePath.h>

#include <rapidjson/document.h>

#define setOptional(expr)                \
    try { expr; }                        \
    catch (const RapidJsonException&) { }

namespace SVE
{
namespace
{
namespace rj = rapidjson;

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

    return engineSettings;
}

std::vector<UniformInfo> getUniformInfoList(rj::Document& document)
{
    static const std::map<std::string, UniformType> uniformMap{
            {"ModelMatrix",               UniformType::ModelMatrix},
            {"ViewMatrix",                UniformType::ViewMatrix},
            {"ProjectionMatrix",          UniformType::ProjectionMatrix},
            {"ModelViewProjectionMatrix", UniformType::ModelViewProjectionMatrix},
            {"CameraPosition",            UniformType::CameraPosition},
            {"LightPosition",             UniformType::LightPosition},
            {"LightColor",                UniformType::LightColor},
            {"LightAmbient",              UniformType::LightAmbient},
            {"LightDiffuse",              UniformType::LightDiffuse},
            {"LightSpecular",             UniformType::LightSpecular},
            {"LightShininess",            UniformType::LightShininess},
            {"LightViewProjection",       UniformType::LightViewProjection},
            {"BoneMatrices",              UniformType::BoneMatrices},
            {"ClipPlane",                 UniformType::ClipPlane},
            {"Time",                      UniformType::Time},
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

VertexInfo getVertexInfo(rj::Document& document)
{
    static const std::map<std::string, VertexInfo::VertexDataType> vertexDataTypeMap{
            {"Position",    VertexInfo::VertexDataType::Position},
            {"Color",       VertexInfo::VertexDataType::Color},
            {"TexCoord",    VertexInfo::VertexDataType::TexCoord},
            {"Normal",      VertexInfo::VertexDataType::Normal},
            {"BoneWeights", VertexInfo::VertexDataType::BoneWeights},
            {"BoneIds",     VertexInfo::VertexDataType::BoneIds},
    };

    VertexInfo info {};

    auto vertexInfo = document["vertexInfo"].GetObject();
    auto vertexDataFlags = vertexInfo["vertexDataFlags"].GetArray();

    info.vertexDataFlags = 0;
    for (auto& item : vertexDataFlags)
    {
        info.vertexDataFlags |= vertexDataTypeMap.at(item.GetString());
    }

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
            {"GeometryShader", ShaderType::GeometryShader}
    };

    rj::Document document;
    document.Parse(data.c_str());

    ShaderSettings shaderSettings {};
    shaderSettings.name = document["name"].GetString();
    setOptional(shaderSettings.maxBonesSize = document["maxBonesSize"].GetUint());
    setOptional(shaderSettings.uniformList = getUniformInfoList(document));
    setOptional(shaderSettings.vertexInfo = getVertexInfo(document));
    setOptional(shaderSettings.samplerNamesList = getStringList(document, "samplerNamesList"));
    shaderSettings.filename = directory.resolve(document["filename"].GetString()).fullPath();
    shaderSettings.shaderType = shaderTypeMap.at(document["shaderType"].GetString());
    setOptional(shaderSettings.entryPoint = document["entryPoint"].GetString());

    return shaderSettings;
}

std::vector<TextureInfo> getTextureInfos(const cppfs::FilePath& directory, rj::Document& document)
{
    auto list = document["textures"].GetArray();
    std::vector<TextureInfo> textureInfosList;

    for (auto& item : list)
    {
        TextureInfo textureInfo {};
        // TODO: Revise shadowmap settings (move it to texture type instead of filename)
        std::string filename = item["filename"].GetString();

        static const std::vector<std::string> usedNames = {
                "shadowmap",
                "reflection",
                "refraction"
        };

        textureInfo.filename = directory.resolve(filename).fullPath();
        for (auto& name : usedNames)
        {
            if (name == filename)
            {
                textureInfo.filename = name;
                break;
            }
        }
        textureInfo.samplerName = item["samplerName"].GetString();

        textureInfosList.push_back(std::move(textureInfo));
    }

    return textureInfosList;
}

MaterialSettings loadMaterial(const cppfs::FilePath& directory, const std::string& data)
{
    rj::Document document;
    document.Parse(data.c_str());

    MaterialSettings materialSettings {};
    materialSettings.name = document["name"].GetString();
    setOptional(materialSettings.invertCullFace = document["invertCullFace"].GetBool());
    setOptional(materialSettings.useDepthTest = document["useDepthTest"].GetBool());
    setOptional(materialSettings.useDepthBias = document["useDepthBias"].GetBool());
    setOptional(materialSettings.useMultisampling = document["useMultisampling"].GetBool());
    setOptional(materialSettings.isCubemap = document["isCubemap"].GetBool());
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

    return meshLoadSettings;
}

glm::vec3 loadVector3(rj::Document& document, const std::string& name)
{
    auto vecArray = document[name.c_str()].GetArray();

    glm::vec3 v;
    for (auto i = 0u; i < 3; i++)
    {
        v[i] = vecArray[i].GetFloat();
    }

    return v;
}

LightSettings loadLight(const std::string& data)
{
    rj::Document document;
    document.Parse(data.c_str());

    LightSettings lightSettings {};

    lightSettings.lightColor = loadVector3(document, "lightColor");
    lightSettings.shininess = document["shininess"].GetFloat();
    lightSettings.ambientStrength = document["ambientStrength"].GetFloat();
    lightSettings.specularStrength = document["specularStrength"].GetFloat();
    lightSettings.diffuseStrength = document["diffuseStrength"].GetFloat();

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
        std::shared_ptr<SVE::Mesh> mesh = std::make_shared<SVE::Mesh>(meshLoadSettings.name, meshLoadSettings.filename);
        engine->getMeshManager()->registerMesh(mesh);
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
        {"light", ResourceType::Light}
    };

    if (resourceTypeMap.find(type) == resourceTypeMap.end())
    {
        // TODO: Add logging system
        std::cout << "Skipping unsupported file " << filename << std::endl;
    }

    std::string fileContent = file.readFile();
    auto directory = cppfs::FilePath(fp.directoryPath());

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
    }
}

} // namespace SVE