// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <string>
#include <vector>

namespace SVE
{
struct MaterialSettings;
struct EngineSettings;
struct ShaderSettings;
struct MeshLoadSettings;
struct LightSettings;

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
    };

    explicit ResourceManager(std::vector<std::string> folderList);
    ResourceManager();

    void loadFolder(const std::string& folder);
    static LoadData getLoadDataFromFolder(const std::string& folder);
    const std::vector<std::string> getFolderList() const;

private:
    enum class ResourceType : uint8_t
    {
        Engine,
        Shader,
        Material,
        Mesh,
        Light,
    };

private:
    void loadResources();
    void initializeResources(LoadData& loadData);

    static void loadDirectory(const std::string& directory, LoadData& loadData);
    static void loadFile(const std::string& filename, LoadData& loadData);

private:

    std::vector<std::string> _folderList;

};

} // namespace SVE