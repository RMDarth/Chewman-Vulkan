// Chewman Vulkan game
// Copyright (c) 2018-2020, Igor Barinov
// Licensed under the MIT License
#include "LocaleManager.h"

#include <utility>
#include <rapidjson/document.h>
#include <SVE/Engine.h>
#include <SVE/ResourceManager.h>
#include <SVE/VulkanException.h>

namespace Chewman
{

const std::string UnknownValue = {};

LocaleManager::LocaleManager(std::string  language)
    : _language(std::move(language))
{
    load();
}

const std::string& LocaleManager::getLocalizedString(const std::string& key) const
{
    auto value = _localeData.find(key);
    if (value != _localeData.end())
        return value->second;

    return UnknownValue;
}

void LocaleManager::load()
{
    namespace rj = rapidjson;

    auto fileSystem = SVE::Engine::getInstance()->getResourceManager()->getFileSystem();
    auto langFile = fileSystem->getEntity("resources/translations/" + _language + ".lang");
    if (!langFile->exist())
    {
        langFile = fileSystem->getEntity("resources/translations/en.lang");
        if (!langFile->exist())
        {
            throw SVE::VulkanException("Can't load localization files");
        }
    }

    auto data = fileSystem->getFileContent(langFile);

    rj::Document document;
    document.Parse(data.c_str());

    for (auto element = document.MemberBegin(); element != document.MemberEnd(); ++element)
        _localeData[element->name.GetString()] = element->value.GetString();
}

const std::string& LocaleManager::getLanguage() const
{
    return _language;
}

} // namespace Chewman