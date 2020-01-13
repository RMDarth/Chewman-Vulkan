// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SystemApi.h"
#include <SDL2/SDL.h>
#ifdef __ANDROID__
#include <jni.h>
#endif

namespace Chewman
{
namespace System
{

#ifdef __ANDROID__
void callVoidMethod(const std::string& name)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, name.c_str(), "()V");

    env->CallVoidMethod(activity, methodId);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
}

bool callBoolMethod(const std::string& name)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, name.c_str(), "()Z");

    bool result = env->CallBooleanMethod(activity, methodId);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return result;
}
#endif

int getSystemVersion()
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "getAndroidVersion", "()I");

    int version = env->CallIntMethod(activity, methodId);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return version;
#endif
    return 0;
}

bool acceptQuary(const std::string& message, const std::string& title, const std::string& accept, const std::string& decline)
{
    const SDL_MessageBoxButtonData buttons[] = {
            { 0, 0, accept.c_str()},
            { 0, 1, decline.c_str()}
    };

    const SDL_MessageBoxData messageBoxData = {
            SDL_MESSAGEBOX_INFORMATION, /* .flags */
            nullptr, /* .window */
            title.c_str(), /* .title */
            message.c_str(), /* .message */
            SDL_arraysize(buttons), /* .numbuttons */
            buttons, /* .buttons */
            nullptr
    };
    int buttonId;
    if (SDL_ShowMessageBox(&messageBoxData, &buttonId) < 0) {
        SDL_Log("Error displaying message box");
        return false;
    }

    return !(buttonId == -1 || buttonId == 1);
}

void restartApp()
{
#ifdef __ANDROID__
    callVoidMethod("restartApp");
#else
    // TODO: Add Desktop restart functionality
    exit(0);
#endif
}

void exitApp()
{
    SDL_Quit();
    exit(0);
}

// This is temporary to test buying functionality on Desktop build
bool isBought = false;
bool isLogged = false;

void buyItem(const std::string& id)
{
#ifdef __ANDROID__
    if (id == "levels")
        callVoidMethod("purchaseCampaign");
#else
    isBought = !isBought;
#endif
}

bool isItemBought(const std::string& id)
{
#ifdef __ANDROID__
    if (id == "levels")
        return callBoolMethod("isCampaignPurchased");
#else
    return isBought;
#endif
}

void initServices()
{

}

void logInServices()
{
#ifdef __ANDROID__
    callVoidMethod("logInGoogle");
#else
    isLogged = true;
#endif
}

void logOutServices()
{
#ifdef __ANDROID__
    callVoidMethod("logOutGoogle");
#else
    isLogged = false;
#endif
}

bool isLoggedServices()
{
#ifdef __ANDROID__
    return callBoolMethod("isGoogleLogged");
#else
    return isLogged;
#endif
}

bool isSignedServices()
{
#ifdef __ANDROID__
    return callBoolMethod("isGoogleEverSigned");
#else
    return false;
#endif
}

bool isServicesAvailable()
{
#ifdef __ANDROID__
    return callBoolMethod("isGoogleServicesAvailable");
#else
    return true;
#endif
}

void showLeaderboard(bool weekly)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "showLeaderboard", "(Z)V");

    env->CallVoidMethod(activity, methodId, weekly);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

void showTimeLeaderboard(int level, bool weekly)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "showTimeLeaderboard", "(IZ)V");

    env->CallVoidMethod(activity, methodId, level, weekly);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

void showAds(AdHorizontalLayout horizontalLayout, AdVerticalLayout verticalLayout)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "showAds", "(II)V");

    env->CallVoidMethod(activity, methodId, static_cast<int>(horizontalLayout), static_cast<int>(verticalLayout));

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

void hideAds()
{
#ifdef __ANDROID__
    callVoidMethod("hideAds");
#endif
}

bool showVideoAds()
{
#ifdef __ANDROID__
    return callBoolMethod("showVideoAds");
#else
    return false;
#endif
}

bool wasVideoAdsWatched()
{
#ifdef __ANDROID__
    return callBoolMethod("wasVideoAdsWatched");
#else
    return false;
#endif
}

std::string achivementTypeToId(AchievementType type)
{
    switch (type)
    {
        case AchievementType::Newcomer: return  "achievement_newcomer";
        case AchievementType::Good_start: return  "achievement_good_start";
        case AchievementType::Treasure_hunter: return  "achievement_treasure_hunter";
        case AchievementType::Dungeon_master: return  "achievement_dungeon_master";
        case AchievementType::Dungeon_emperor: return  "achievement_dungeon_emperor";
        case AchievementType::Star_collector: return  "achievement_star_collector";
        case AchievementType::Star_hunter: return  "achievement_star_hunter";
        case AchievementType::Supernova: return  "achievement_supernova";
        case AchievementType::Gold_rush: return  "achievement_gold_rush";
        case AchievementType::Mission_accomplished: return  "achievement_mission_accomplished";
    }

    return "";
}

void unlockAchievement(AchievementType achievementType)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "unlockAchievement", "(Ljava/lang/String;)V");

    // Strings passed to the function need to be converted to a java string object
    auto achievementId = achivementTypeToId(achievementType);
    jstring jstr = env->NewStringUTF(achievementId.c_str());

    env->CallVoidMethod(activity, methodId, jstr);

    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

void updateAchievement(AchievementType achievementType, int score)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "updateAchievement", "(Ljava/lang/String;I)V");

    auto achievementId = achivementTypeToId(achievementType);
    jstring jstr = env->NewStringUTF(achievementId.c_str());

    env->CallVoidMethod(activity, methodId, jstr, score);

    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

void showAchievementUI()
{
#ifdef __ANDROID__
    callVoidMethod("showAchievementUI");
#endif
}

bool isScoresUpdated()
{
#ifdef __ANDROID__
    return callBoolMethod("isScoresUpdated");
#else
    return false;
#endif
}

void updateScores()
{
#ifdef __ANDROID__
    callVoidMethod("getTopScores");
    callVoidMethod("getTopTimeScores");
#endif
}

void refreshScores()
{
#ifdef __ANDROID__
    callVoidMethod("refreshScores");
#endif
}

void submitScore(int score, bool best)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "submitScore", "(IZ)V");

    env->CallVoidMethod(activity, methodId, score, best);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

void submitLevelTime(int level, int timeSeconds, bool best)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "submitTimeScore", "(IIZ)V");

    env->CallVoidMethod(activity, methodId, level, timeSeconds, best);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
}

std::vector<bool> getIsScoresSubmitted()
{
    std::vector<bool> scores(37, true);

#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);
    
    jmethodID methodIdName = env->GetMethodID(activityClass, "getSubmitState", "()[Z");

    auto resultArray = (jbooleanArray)env->CallObjectMethod(activity, methodIdName);

    if (resultArray == nullptr)
    {
        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);

        return scores;
    }

    jsize resultSize = env->GetArrayLength(resultArray);
    jboolean* resultElements = env->GetBooleanArrayElements(resultArray, nullptr);

    if (resultElements && resultSize == scores.size())
    {
        for (auto i = 0; i < scores.size(); ++i)
            scores[i] = resultElements[i];
    }

    env->ReleaseBooleanArrayElements(resultArray, resultElements, JNI_ABORT);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif

    return scores;
}

std::vector<std::pair<std::string, int>> getScores(bool weekly)
{
    std::vector<std::pair<std::string, int>> scoresList;
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodIdName = env->GetMethodID(activityClass, "getTopScoresName", "(IZ)Ljava/lang/String;");
    jmethodID methodIdScore = env->GetMethodID(activityClass, "getTopScoresScore", "(IZ)I");

    for (auto i = 0; i < 5; ++i)
    {
        auto nameObj = (jstring)env->CallObjectMethod(activity, methodIdName, i, weekly);
        if (nameObj == nullptr)
        {
            scoresList.emplace_back("", 0);
            continue;
        }

        const char *name = env->GetStringUTFChars(nameObj, nullptr);
        int score = env->CallIntMethod(activity, methodIdScore, i, weekly);
        scoresList.emplace_back(name, score);

        env->ReleaseStringUTFChars(nameObj, name);
        env->DeleteLocalRef(nameObj);
    }

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
    return scoresList;
}

int getPlayerPlace(bool weekly)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "getPlayerPlace", "(Z)I");

    int result = env->CallIntMethod(activity, methodId, weekly);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return result;
#else
    return 10;
#endif
}

std::pair<std::string, int> getPlayerScore(bool weekly)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodIdName = env->GetMethodID(activityClass, "getPlayerName", "(Z)Ljava/lang/String;");
    jmethodID methodIdScore = env->GetMethodID(activityClass, "getPlayerScore", "(Z)I");

    auto nameObj = (jstring)env->CallObjectMethod(activity, methodIdName, weekly);

    if (nameObj != nullptr) {
        const char *name = env->GetStringUTFChars(nameObj, nullptr);
        int score = env->CallIntMethod(activity, methodIdScore, weekly);

        std::pair<std::string, int> result = {name, score};

        env->ReleaseStringUTFChars(nameObj, name);
        env->DeleteLocalRef(nameObj);

        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);

        return result;
    } else {
        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);

        return {"Player", 0};
    }
#else
    return {"Player", 0};
#endif
}

std::string getPlayerName()
{
#ifdef __ANDROID__

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodIdName = env->GetMethodID(activityClass, "getPlayerDisplayName", "()Ljava/lang/String;");

    auto nameObj = (jstring)env->CallObjectMethod(activity, methodIdName);
    if (nameObj != nullptr) {
        const char *name = env->GetStringUTFChars(nameObj, nullptr);

        std::string result = name;

        env->ReleaseStringUTFChars(nameObj, name);

        env->DeleteLocalRef(nameObj);
        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);

        return result;
    } else {
        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);

        return "Player";
    }
#else
    return "Player";
#endif
}

std::vector<std::pair<std::string, int>> getTimes(bool weekly)
{
    std::vector<std::pair<std::string, int>> scoresList;
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodIdName = env->GetMethodID(activityClass, "getTopTimeScoresName", "(IZ)Ljava/lang/String;");
    jmethodID methodIdScore = env->GetMethodID(activityClass, "getTopTimeScoresValue", "(IZ)I");

    for (auto i = 0; i < 36; ++i)
    {
        auto nameObj = (jstring)env->CallObjectMethod(activity, methodIdName, i, weekly);
        if (nameObj == nullptr)
        {
            scoresList.emplace_back("", 0);
        } else {
            const char *name = env->GetStringUTFChars(nameObj, nullptr);
            int version = env->CallIntMethod(activity, methodIdScore, i, weekly);
            scoresList.emplace_back(name, version);

            env->ReleaseStringUTFChars(nameObj, name);
            env->DeleteLocalRef(nameObj);
        }
    }

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#endif
    return scoresList;
}

bool hasNewTimeScores()
{
#ifdef __ANDROID__
    return callBoolMethod("hasNewTimeScoreChanges");
#else
    return false;
#endif
}

void requestBackup()
{

}

void requestRestore()
{

}

#ifdef __ANDROID__
std::vector<uint8_t> callByteArrayMethod(const std::vector<uint8_t>& data, const char* const name)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jbyteArray byteArray = env->NewByteArray(data.size());
    env->SetByteArrayRegion(byteArray, 0, data.size(), reinterpret_cast<const jbyte*>(data.data()));

    jmethodID methodIdName = env->GetMethodID(activityClass, name, "([B)[B");

    auto resultArray = (jbyteArray)env->CallObjectMethod(activity, methodIdName, byteArray);

    if (resultArray == nullptr) {
        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);
        return std::vector<uint8_t>();
    }

    jsize resultSize = env->GetArrayLength(resultArray);
    jbyte* resultElements = env->GetByteArrayElements(resultArray, nullptr);

    if (!resultElements)
        resultSize = 0;

    std::vector<uint8_t> result(resultSize);

    if (resultSize > 0)
        std::memcpy(result.data(), resultElements, resultSize);

    env->ReleaseByteArrayElements(resultArray, resultElements, JNI_ABORT);

    env->DeleteLocalRef(byteArray);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return result;
}
#endif

std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data)
{
#ifdef __ANDROID__
    return callByteArrayMethod(data, "encrypt");

#else
    // TODO: Add some basic encryption
    return data;
#endif
}

std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data)
{
#ifdef __ANDROID__
    return callByteArrayMethod(data, "decrypt");
#else
    return data;
#endif
}

void syncAchievements()
{

}

void rateApp()
{

}

void openLink(const std::string& link)
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "openLink", "(Ljava/lang/String;)V");
    jstring jstr = env->NewStringUTF(link.c_str());

    env->CallVoidMethod(activity, methodId, jstr);

    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#else
    std::string openCommand =
#ifdef WIN32
        "start ";
#elif __linux__
        "xdg-open ";
#else
        "open ";
#endif
    system((openCommand + link).c_str());
#endif
}

bool hasTimeScoresUpdated()
{
#ifdef __ANDROID__
    return callBoolMethod("hasTimescoresUpdated");
#else
    return false;
#endif
}

} // namespace System
} // namespace Chewman