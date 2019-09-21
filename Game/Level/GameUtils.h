// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <random>
#include <glm/vec3.hpp>

namespace Chewman
{

std::mt19937& getRandomEngine();

glm::vec3 getWorldPos(int row, int column, float y = 0.0f);

} // namespace Chewman