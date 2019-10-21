// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "FileSystem.h"

namespace SVE
{
struct MaterialSettings;
struct EngineSettings;
struct ShaderSettings;
struct MeshLoadSettings;
struct LightSettings;
struct ParticleSystemSettings;
struct Font;

class ResourceManager
{
public:
    struct LoadData
    {
        std::vector<MaterialSettings> materialsList;
        std::vector<EngineSettings> engine;
        std::vector<ShaderSettings> shaderList;
        std::vector<MeshLoadSettings> meshList;
        std::vector<LightSettings> lightList;
        std::vector<ParticleSystemSettings> particleSystemList;
        std::vector<Font> fontList;
    };

    explicit ResourceManager(std::shared_ptr<FileSystem> fileSystem);

    void loadFolder(const std::string& folder);
    static LoadData getLoadDataFromFolder(const std::string& folder, bool isFolder, const std::shared_ptr<FileSystem>& fileSystem);
    const std::vector<std::string> getFolderList() const;
    std::string loadFileContent(const std::string& file) const;

private:
    enum class ResourceType : uint8_t
    {
        Engine,
        Shader,
        Material,
        Mesh,
        Light,
        ParticleSystem,
        Font
    };

private:
    void loadResources();
    void initializeResources(LoadData& loadData);

    static void loadDirectory(const std::string& directory, LoadData& loadData, const std::shared_ptr<FileSystem>& fileSystem);
    static void loadFile(FSEntityPtr file, LoadData& loadData, const std::shared_ptr<FileSystem>& fileSystem);

private:
    std::vector<std::string> _folderList;
    std::shared_ptr<FileSystem> _fileSystem;
};

} // namespace SVE