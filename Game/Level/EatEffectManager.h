// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "SVE/SceneNode.h"
#include <queue>
#include <array>

namespace Chewman
{
class GameMap;

enum class EatEffectType : uint8_t
{
    Gold,
    Walls
};

constexpr uint32_t EatEffectsCount = 2;

class EatEffectManager
{
public:
    explicit EatEffectManager(GameMap* gameMap);

    void addEffect(EatEffectType type, glm::ivec2 pos);
    void update(float deltaTime);

private:
    struct EffectInfo
    {
        float time;
        std::shared_ptr<SVE::SceneNode> effectNode;
        EatEffectType type;
    };

    GameMap* _gameMap;
    std::list<EffectInfo> _currentEffects;
    std::array<std::queue<std::shared_ptr<SVE::SceneNode>>, EatEffectsCount> _effectsPool;
    bool _isParticlesEnabled = true;
};

} // anonymous Chewman