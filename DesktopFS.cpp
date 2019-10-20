// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "DesktopFS.h"
#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>
#include <cppfs/FilePath.h>

namespace SVE
{

DesktopFSEntity::DesktopFSEntity(cppfs::FileHandle handle)
    : Handle(std::move(handle))
{
}

bool DesktopFSEntity::isDirectory() const
{
    return Handle.isDirectory();
}

std::string DesktopFSEntity::getPath() const
{
    return Handle.path();
}

std::string DesktopFSEntity::resolveFilePath(const std::string& file) const
{
    return cppfs::FilePath(Handle.path()).resolve(file).fullPath();
}

bool DesktopFSEntity::exist() const
{
    return Handle.exists();
}

std::string DesktopFS::getExtension(std::shared_ptr<FileSystemEntity> file)
{
    cppfs::FilePath fp(file->getPath());
    return fp.extension();
}

FSEntityPtr DesktopFS::getContainingDirectory(std::shared_ptr<FileSystemEntity> file)
{
    cppfs::FilePath fp(file->getPath());
    return getEntity(fp.directoryPath());
}

FSEntityList DesktopFS::getFileList(std::shared_ptr<FileSystemEntity> dir)
{
    FSEntityList fileList;
    if (!dir->isDirectory())
        return fileList;

    auto dirHandle = std::static_pointer_cast<DesktopFSEntity>(dir)->Handle;
    cppfs::FilePath fp(dirHandle.path());
    for (cppfs::FileIterator it = dirHandle.begin(); it != dirHandle.end(); ++it)
    {

        fileList.push_back(std::make_shared<DesktopFSEntity>(cppfs::fs::open(fp.resolve(*it).fullPath())));
    }

    return fileList;
}

std::string DesktopFS::getFileContent(std::shared_ptr<FileSystemEntity> file)
{
    return std::static_pointer_cast<DesktopFSEntity>(file)->Handle.readFile();
}

std::shared_ptr<FileSystemEntity> DesktopFS::getEntity(const std::string& localPath, bool /*isWritable*/)
{
    return std::make_shared<DesktopFSEntity>(cppfs::fs::open(localPath));
}
} // namespace SVE