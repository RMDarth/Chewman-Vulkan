// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <string>

namespace Chewman
{
namespace System
{

int getSystemVersion();
bool acceptQuary(const std::string& message, const std::string& title, const std::string& accept, const std::string& decline);
void restartApp();

} // namespace System
} // namespace Chewman
