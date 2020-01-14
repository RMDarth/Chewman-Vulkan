// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include <SVE/VulkanException.h>
#include <SDL2/SDL_filesystem.h>
#include "AndroidFS.h"
#include <android/asset_manager.h>
#include <chrono>
#include <iostream>

namespace SVE
{

AndroidFS::AndroidFS(AAssetManager *mgr)
    : _assetManager(mgr)
{
}

std::string AndroidFS::getExtension(FSEntityPtr file) const
{
    auto path = file->getPath();
    auto it = path.find_last_of('.');
    if (it ==  std::string::npos)
        return std::string();
    else
        return path.substr(it);
}

FSEntityPtr AndroidFS::getContainingDirectory(FSEntityPtr file) const
{
    auto path = file->getPath();

    FSEntityPtr entity;
    auto it = path.find_last_of('/');
    if (it ==  std::string::npos) {
        auto cached = _dirCache.find("/");
        if (cached != _dirCache.end())
            return cached->second;
        entity = getEntity("/", true);
        _dirCache["/"] = entity;
    } else {
        auto dirPath = path.substr(0, it);
        auto cached = _dirCache.find(dirPath);
        if (cached != _dirCache.end())
            return cached->second;
        entity = getEntity(dirPath, true);
        _dirCache[dirPath] = entity;
    }

    return entity;
}

FSEntityList AndroidFS::getFileList(FSEntityPtr dir) const
{
    FSEntityList list;

    if (!dir->isDirectory())
        return list;

    auto* dirHandle = std::static_pointer_cast<AndroidFSEntity>(dir)->Dir;
    while (const auto* path = AAssetDir_getNextFileName(dirHandle))
    {
        list.push_back(getEntity(dir->resolveFilePath(path)));
    }

    return list;
}

std::string AndroidFS::getFileContent(FSEntityPtr file) const
{
    auto* fileHandle = std::static_pointer_cast<AndroidFSEntity>(file)->Handle;
    auto length = AAsset_getLength(fileHandle);

    char *data = new char[length + 1];
    AAsset_read(fileHandle, data, length);
    std::string result(data, data + length);
    delete[] data;

    return result;
}

FSEntityPtr AndroidFS::getEntity(const std::string& localPath, bool isDirectory) const
{
    return std::make_shared<AndroidFSEntity>(localPath, isDirectory, _assetManager);
}

std::string AndroidFS::getSavePath() const
{
    char *path = SDL_GetPrefPath("TurbulentSoftware", "Chewman");
    if (!path)
    {
        throw VulkanException("Can't get folder for saving data");
    }
    return std::string(path);
}

AndroidFSEntity::AndroidFSEntity(std::string path, bool isDir, AAssetManager* assetManager)
    : _path(std::move(path))
    , _isDirectory(isDir)
{
    //auto time = std::chrono::high_resolution_clock::now();
    if (isDir)
    {
        Dir = AAssetManager_openDir(assetManager, _path.c_str());
        //auto duration = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - time).count() * 1000.0f;
        //std::cout << "Opening dir " << _path<< " took " << duration << std::endl;
    } else {
#ifdef FLATTEN_FS

        auto fixedPath = "resflat" + _path.substr(_path.find_last_of('/'));
        Handle = AAssetManager_open(assetManager, fixedPath.c_str(), AASSET_MODE_BUFFER);

#else
        Handle = AAssetManager_open(assetManager, _path.c_str(), AASSET_MODE_BUFFER);
#endif
        //auto duration = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - time).count() * 1000.0f;
        //std::cout << "Opening file " << _path << " took " << duration << std::endl;
    }
}

AndroidFSEntity::~AndroidFSEntity()
{
    if (!_isDirectory)
        AAsset_close(Handle);
    else
        AAssetDir_close(Dir);

}
bool AndroidFSEntity::isDirectory() const
{
    return _isDirectory;
}

bool AndroidFSEntity::exist() const
{
    return true;
}

std::string AndroidFSEntity::getPath() const
{
    return _path;
}

std::string AndroidFSEntity::resolveFilePath(const std::string& file) const
{
    auto result = _path + "/" + file;
    auto it = result.find("/..");
    while (it != std::string::npos)
    {
        if (it == 0)
            return result;
        for (int i = it - 1; i >= 0; i--)
        {
            if (result[i] == '/')
            {
                result = result.substr(0, i) + result.substr(it + 3);
                break;
            }
        }

        it = result.find("/..");
    }
    return result;
}

} // namespace SVE