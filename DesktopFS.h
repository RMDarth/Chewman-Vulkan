// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include <cppfs/FileHandle.h>
#include "SVE/FileSystem.h"

namespace SVE
{

class DesktopFSEntity : public FileSystemEntity
{
public:
    explicit DesktopFSEntity(cppfs::FileHandle handle);

    bool isDirectory() const override;
    bool exist() const override;
    std::string getPath() const override;
    std::string resolveFilePath(const std::string& file) const override;

    cppfs::FileHandle Handle;
};

class DesktopFS : public FileSystem
{
public:
    std::string getExtension(FSEntityPtr file) const override;
    FSEntityPtr getContainingDirectory(FSEntityPtr file) const override;
    FSEntityList getFileList(FSEntityPtr dir) const override;
    std::string getFileContent(FSEntityPtr file) const override;
    FSEntityPtr getEntity(const std::string& localPath, bool isDirectory = false) const override;
    std::string getSavePath() const override;
};

} // namespace SVE