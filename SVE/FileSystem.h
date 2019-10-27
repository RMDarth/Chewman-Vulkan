// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include <string>
#include <vector>
#include <memory>

namespace SVE
{

class FileSystemEntity
{
public:
    virtual ~FileSystemEntity() = default;

    virtual bool isDirectory() const = 0;
    virtual bool exist() const = 0;
    virtual std::string getPath() const = 0;
    virtual std::string resolveFilePath(const std::string& file) const = 0;
};

using FSEntityList = std::vector<std::shared_ptr<FileSystemEntity>>;
using FSEntityPtr = std::shared_ptr<FileSystemEntity>;

class FileSystem
{
public:
    virtual ~FileSystem() = default;

    virtual std::string getExtension(FSEntityPtr file) const = 0;
    virtual FSEntityPtr getContainingDirectory(FSEntityPtr file) const = 0;
    virtual FSEntityList getFileList(FSEntityPtr dir) const = 0;
    virtual std::string getFileContent(FSEntityPtr file) const = 0;
    virtual std::string getSavePath() const = 0;

    virtual FSEntityPtr getEntity(const std::string& localPath, bool isDirectory = false) const = 0;
};

} // namespace SVE