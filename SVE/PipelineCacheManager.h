// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace SVE
{

class PipelineCacheManager
{
public:
    PipelineCacheManager();

    const std::vector<uint8_t>& getCache(const std::string& materialName) const;
    void addCache(const std::string& materialName, std::vector<uint8_t> cache);

    void load();
    void store();
    void reset();

    bool isNew() const;

private:
    bool _empty = true;
    std::vector<uint8_t> _emptyData;
    std::unordered_map<std::string, std::vector<uint8_t>> _cacheMap;
};

} // namespace SVE