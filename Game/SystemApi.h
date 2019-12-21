// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <string>
#include <vector>

namespace Chewman
{
namespace System
{

const std::string levelsProduct = "levels";

int getSystemVersion();
bool acceptQuary(const std::string& message, const std::string& title, const std::string& accept, const std::string& decline);
void restartApp();
void exitApp();

void buyItem(const std::string& id);
bool isItemBought(const std::string& id);
void initServices();

void logInServices();
void logOutServices();
bool isLoggedServices();
bool isServicesAvailable();

void showAchievements();
void showLeaderboard();

bool isScoresUpdated();
void updateScore(int score);
void updateLevelTime(int level, int timeSeconds);
std::vector<std::pair<std::string, int>> getScores(bool weekly);
std::vector<std::pair<std::string, int>> getTimes(bool weekly);

bool showVideoAds();
void requestBackup();
void requestRestore();

void syncAchievements();
void rateApp();

void openLink(const std::string& link);

} // namespace System
} // namespace Chewman
