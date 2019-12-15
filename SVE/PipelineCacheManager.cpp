// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "PipelineCacheManager.h"
#include "Engine.h"
#include "ResourceManager.h"
#include <fstream>
#include <utility>

namespace SVE
{

namespace
{

constexpr uint32_t CacheFileVersion = 1;

std::string getCachePath()
{
    auto cachePath = SVE::Engine::getInstance()->getResourceManager()->getSavePath();
    if (cachePath.back() != '/' || cachePath.back() != '\\')
        cachePath.push_back('/');
    cachePath += "Pipeline.cache";
    return cachePath;
}

} // anon namespace

PipelineCacheManager::PipelineCacheManager() = default;

const std::vector<uint8_t>& PipelineCacheManager::getCache(const std::string& materialName) const
{
    if (_empty)
        return _emptyData;
    auto cachePos = _cacheMap.find(materialName);
    if (cachePos == _cacheMap.end())
        return _emptyData;
    return cachePos->second;
}

void PipelineCacheManager::addCache(const std::string& materialName, std::vector<uint8_t> cache)
{
    _cacheMap[materialName] = std::move(cache);
}

void PipelineCacheManager::load()
{
    std::ifstream fin(getCachePath());
    if (!fin)
    {
        std::cout << "No cache exists or can't open cache file" << std::endl;
        _empty = true;
        return;
    }

    uint32_t version = 0;
    fin.read(reinterpret_cast<char*>(&version), sizeof(CacheFileVersion));
    if (version != CacheFileVersion)
    {
        _empty = true;
        return;
    }


    uint32_t elementsCount = 0;
    fin.read(reinterpret_cast<char*>(&elementsCount), sizeof(elementsCount));
    if (elementsCount == 0)
        return;

    _empty = false;
    for (auto i = 0; i < elementsCount; ++i)
    {
        uint32_t nameSize = 0;
        fin.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
        std::vector<char> nameValue(nameSize);
        fin.read(reinterpret_cast<char*>(nameValue.data()), nameSize);
        std::string name(nameValue.begin(), nameValue.end());
        uint32_t dataSize = 0;
        fin.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
        std::vector<uint8_t> data(dataSize);
        fin.read(reinterpret_cast<char*>(data.data()), dataSize);

        _cacheMap[name] = data;
    }

    fin.close();
    std::cout << "Pipeline cache loaded. " << std::endl;
}

void PipelineCacheManager::store()
{
    std::ofstream fout(getCachePath());
    if (!fout)
    {
        std::cout << "Can't create pipeline cache file" << std::endl;
        return;
    }

    fout.write(reinterpret_cast<const char*>(&CacheFileVersion), sizeof(CacheFileVersion));
    uint32_t elementsCount = _cacheMap.size();
    fout.write(reinterpret_cast<const char*>(&elementsCount), sizeof(elementsCount));
    for (auto &element : _cacheMap)
    {
        uint32_t nameSize = element.first.size();
        fout.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
        fout.write(reinterpret_cast<const char*>(element.first.data()), nameSize);
        uint32_t dataSize = element.second.size();
        fout.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        fout.write(reinterpret_cast<const char*>(element.second.data()), dataSize);
    }

    fout.close();
}

bool PipelineCacheManager::isNew() const
{
    return _empty;
}

void PipelineCacheManager::reset()
{
    _cacheMap.clear();
    _empty = true;
}
} // namespace SVE