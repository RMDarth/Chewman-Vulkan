// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <string>
#include <vector>

namespace Chewman
{
namespace System
{

enum class AdHorizontalLayout : int
{
    Left = 9,
    Center = 14,
    Right = 11
};

enum class AdVerticalLayout : int
{
    Top = 10,
    Bottom = 12
};

enum class AchievementType : int
{
    Newcomer,
    Good_start,
    Treasure_hunter,
    Dungeon_master,
    Dungeon_emperor,
    Star_collector,
    Star_hunter,
    Supernova,
    Gold_rush,
    Mission_accomplished,
};

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
bool isSignedServices();
bool isServicesAvailable();

void showAchievements();
void showLeaderboard();

void showAds(AdHorizontalLayout horizontalLayout, AdVerticalLayout verticalLayout);
void hideAds();

void unlockAchievement(AchievementType achievementType);
void updateAchievement(AchievementType achievementType, int score);
void showAchievementUI();

bool isScoresUpdated();
bool hasNewTimeScores();
void updateScores();
void refreshScores();
void submitScore(int score);
void submitLevelTime(int level, int timeSeconds);
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
