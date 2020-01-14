// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include "SVE/FileSystem.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unordered_map>

//#define FLATTEN_FS

namespace SVE
{

class AndroidFSEntity : public FileSystemEntity
{
public:
    AndroidFSEntity(std::string path, bool isDir, AAssetManager* assetManager);
    ~AndroidFSEntity();

    bool isDirectory() const override;
    bool exist() const override;
    std::string getPath() const override;
    std::string resolveFilePath(const std::string& file) const override;

    AAsset* Handle = nullptr;
    AAssetDir* Dir = nullptr;

private:
    std::string _path;
    bool _isDirectory = false;
};

class AndroidFS : public FileSystem
{
public:
    explicit AndroidFS(AAssetManager *mgr);

    std::string getExtension(FSEntityPtr file) const override;
    FSEntityPtr getContainingDirectory(FSEntityPtr file) const override;
    FSEntityList getFileList(FSEntityPtr dir) const override;
    std::string getFileContent(FSEntityPtr file) const override;
    FSEntityPtr getEntity(const std::string& localPath, bool isDirectory = false) const override;

    std::string getSavePath() const override;

private:
    AAssetManager* _assetManager;
    mutable std::unordered_map<std::string, FSEntityPtr> _dirCache;
};

} // namespace SVE