// Chewman Vulkan game
// Copyright (c) 2018-2020, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <unordered_map>
#include <string>

namespace Chewman
{

class LocaleManager
{
public:
    explicit LocaleManager(std::string language);
    const std::string&  getLocalizedString(const std::string& key) const;

private:
    void load();

    std::string _language;
    std::unordered_map<std::string, std::string> _localeData;
};

} // namespace Chewman