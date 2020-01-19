// Chewman Vulkan game
// Copyright (c) 2018-2020, Igor Barinov
// Licensed under the MIT License
#include "SDL.h"
#include "VulkanHeaders.h"
#include <vector>
#include <android/log.h>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL_vulkan.h>
#include <thread>
#include <future>
#include <chrono>
#include <fstream>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "logging.h"
#include "SVE/Engine.h"
#include "SVE/VulkanException.h"
#include "SVE/SceneManager.h"
#include "SVE/MeshEntity.h"
#include "SVE/ResourceManager.h"
#include "SVE/LightManager.h"
#include "SVE/PostEffectManager.h"
#include "SVE/PipelineCacheManager.h"

#include "Game/Game.h"
#include "Game/Controls/ControlDocument.h"

#include "AndroidFS.h"

#define SVE_ASSERT(message) assert(!message)

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "Chewman", __VA_ARGS__)

SDL_Window *window = NULL;
jobject globalAssetManager;
bool firstRun = false;
bool isMinimized = false;

ANativeWindow* GetNativeWindow()
{
    ANativeWindow *anw = NULL;
    jobject s;
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

    jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass activityClass = env->GetObjectClass(activity);

    jmethodID midGetNativeSurface = env->GetStaticMethodID(activityClass,
                                                    "getNativeSurface","()Landroid/view/Surface;");

    s = env->CallStaticObjectMethod(activityClass, midGetNativeSurface);
    if (s) {
        anw = ANativeWindow_fromSurface(env, s);
        env->DeleteLocalRef(s);
    }

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return anw;
}

Chewman::GraphicsSettings detectSettings(SVE::AndroidFS& androidFS)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "getQuality", "()I");

    int quality = env->CallIntMethod(activity, methodId);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    Chewman::GraphicsSettings settings {};
    switch (quality)
    {
        case 0:
            settings.resolution = Chewman::ResolutionSettings::Low;
            settings.effectSettings = Chewman::EffectSettings::Low;
            settings.dynamicLights = Chewman::LightSettings::Off;
            settings.particleEffects = Chewman::ParticlesSettings::None;
            break;
        case 1:
            settings.resolution = Chewman::ResolutionSettings::Low;
            settings.effectSettings = Chewman::EffectSettings::Medium;
            settings.dynamicLights = Chewman::LightSettings::Simple;
            settings.particleEffects = Chewman::ParticlesSettings::None;
            break;
        case 2:
            settings.resolution = Chewman::ResolutionSettings::High;
            settings.effectSettings = Chewman::EffectSettings::High;
            settings.dynamicLights = Chewman::LightSettings::High;
            break;

        default:
            settings.resolution = Chewman::ResolutionSettings::High;
            settings.effectSettings = Chewman::EffectSettings::High;
            settings.dynamicLights = Chewman::LightSettings::High;
    }

    auto settingsPath = androidFS.getSavePath();
    if (settingsPath.back() != '/' || settingsPath.back() != '\\')
        settingsPath.push_back('/');
    settingsPath += "settings.dat";
    std::ofstream fout(settingsPath);
    fout.write(reinterpret_cast<const char*>(&settings), sizeof(settings));
    fout.close();

    return settings;
}

Chewman::GraphicsSettings getGraphicsSettings(SVE::AndroidFS& androidFS)
{
    Chewman::GraphicsSettings settings {};
    auto settingsPath = androidFS.getSavePath();
    if (settingsPath.back() != '/' || settingsPath.back() != '\\')
        settingsPath.push_back('/');
    settingsPath += "settings.dat";
    std::ifstream fin(settingsPath);
    if (!fin)
    {
        // Load file doesn't exist
        firstRun = true;
        return detectSettings(androidFS);
    }
    fin.read(reinterpret_cast<char*>(&settings), sizeof(settings));
    fin.close();

    return settings;
}

AAssetManager* getAssetManager()
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

    jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass activityClass = env->GetObjectClass(activity);

    jmethodID activityClassGetAssets = env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject assetManager = env->CallObjectMethod(activity, activityClassGetAssets);
    globalAssetManager = env->NewGlobalRef(assetManager);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return AAssetManager_fromJava(env, globalAssetManager);
}

void showAlert(const char* message) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

    jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass activityClass = env->GetObjectClass(activity);

    // Get the ID of the method we want to call
    // This must match the name and signature from the Java side
    // Signature has to match java implementation (second string hints a t a java string parameter)
    jmethodID methodID = env->GetMethodID(activityClass, "showAlert", "(Ljava/lang/String;)V");

    // Strings passed to the function need to be converted to a java string object
    jstring jmessage = env->NewStringUTF(message);

    env->CallVoidMethod(activity, methodID, jmessage);

    // Remember to clean up passed values
    env->DeleteLocalRef(jmessage);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeMinimize(JNIEnv *env, jclass clazz) {
    isMinimized = true;
    //SVE::Engine::getInstance()->finishRendering();
}

glm::ivec2 setResolution(SVE::AndroidFS& androidFS)
{
    auto graphicalSettings = getGraphicsSettings(androidFS);

    auto* nativeWindow = GetNativeWindow();
    auto width = ANativeWindow_getWidth(nativeWindow);
    auto height = ANativeWindow_getHeight(nativeWindow);

    std::cout << "Native size: " << width << " x " << height << std::endl;


    glm::ivec2 resolution;

    switch (graphicalSettings.resolution)
    {
        case Chewman::ResolutionSettings::Low:
        {
            //if (height < 720 && height > 0)
            //    break;
            int heightScale = 720;
            if (height % 768 == 0)
                heightScale = 768;
            ANativeWindow_setBuffersGeometry(nativeWindow, (int)(heightScale * width / (float)height), heightScale, 0);
            resolution = {(int)(heightScale * width / (float)height), heightScale};
            break;
        }
        case Chewman::ResolutionSettings::High:
            //if (height < 1080 && height > 0)
            //    break;
            ANativeWindow_setBuffersGeometry(nativeWindow, (int)(1080.0f * width / (float)height), 1080, 0);
            resolution = {(int)(1080.0f * width / (float)height), 1080};
            break;
        case Chewman::ResolutionSettings::Native:
            break;
        case Chewman::ResolutionSettings::Custom:
            break;
    }

    std::cout << "Final size: " << resolution.x << " x " << resolution.y << std::endl;
    return resolution;
}
 
int SDL_main(int argc, char *argv[]) {

    InitVulkan();
 
    int running = 1;
 
    LOG("started");
    startLogger();
 
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
 
    window = SDL_CreateWindow("Chewman Vulkan",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);

    //SDL_SetWindowSize(window, 1280, 720);
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    try {
        auto androidFS = std::make_shared<SVE::AndroidFS>(getAssetManager());
        auto resolution = setResolution(*androidFS);

        SVE::Engine *engine = SVE::Engine::createInstance(window, "resources/main.engine", androidFS, resolution);
        engine->setIsFirstRun(firstRun);
        auto& graphicsManager = Chewman::GraphicsManager::getInstance();
        std::cout << "Render window size by SDL: " << engine->getRenderWindowSize().x << " " << engine->getRenderWindowSize().y << std::endl;
        auto camera = engine->getSceneManager()->createMainCamera();

        // show loading screen
        engine->getResourceManager()->loadFolder("resources/loadingScreen");
        std::unique_ptr<Chewman::ControlDocument> loadingScreen;
        if ((float)resolution.x / resolution.y < 1.4f)
        {
            loadingScreen = std::make_unique<Chewman::ControlDocument>("resources/game/GUI/loading.xml");
        } else {
            loadingScreen = std::make_unique<Chewman::ControlDocument>("resources/game/GUI/loadingWide.xml");
        }
        engine->renderFrame(0.0f);

        auto progressControl = loadingScreen->getControlByName("progress");
        auto progressSize = loadingScreen->getControlByName("progressAll")->getSize();
        auto updateProgress = [&](float percent)
        {
            progressControl->setSize({progressSize.x * percent, progressSize.y});
            engine->renderFrame();
        };
        auto updateProgressBetween = [&](float start, float end, float percent)
        {
            updateProgress(start + (end-start)*percent);
        };

        auto future = std::async(std::launch::async, [&] {
            std::cout << "Start loading resources..." << std::endl;

            // load resources
            engine->getPipelineCacheManager()->load();
            if (graphicsManager.getSettings().effectSettings == Chewman::EffectSettings::Low)
                engine->getResourceManager()->setMaxMaterialLoadQuality(SVE::MaterialQuality::Low);
            else if (graphicsManager.getSettings().effectSettings == Chewman::EffectSettings::Medium)
                engine->getResourceManager()->setMaxMaterialLoadQuality(SVE::MaterialQuality::Medium);
#ifdef FLATTEN_FS
            engine->getResourceManager()->loadFolder("resflat");
#else
            using namespace std::placeholders;
            updateProgress(0.05f);
            engine->getResourceManager()->loadFolder("resources/shaders", std::bind(updateProgressBetween, 0.05f, 0.15f, _1));
            std::cout << "Shaders loaded." << std::endl;
            updateProgress(0.15f);
            engine->getResourceManager()->loadFolder("resources/materials", std::bind(updateProgressBetween, 0.15f, 0.45f, _1));
            std::cout << "Main materials loaded." << std::endl;
            updateProgress(0.45f);
            //engine->getResourceManager()->loadFolder("resources/materials/skins");
            //std::cout << "Skin materials loaded." << std::endl;
            engine->getResourceManager()->loadFolder("resources/models", std::bind(updateProgressBetween, 0.45f, 0.7f, _1));
            std::cout << "3D assets loaded." << std::endl;
            updateProgress(0.7f);
            engine->getResourceManager()->loadFolder("resources/fonts");
            std::cout << "Fonts loaded." << std::endl;
            updateProgress(0.75f);
            engine->getResourceManager()->loadFolder("resources");
            std::cout << "Particles loaded." << std::endl;

#endif
            updateProgress(0.8f);
            std::cout << "Resources loading finished." << std::endl;

            return 0;
        });

        {
            using namespace std::chrono_literals;
            auto status = future.wait_for(0ms);
            while (status != std::future_status::ready)
            {
                //engine->renderFrame(0.0f);
                SDL_Event event;
                SDL_PollEvent(&event);
                status = future.wait_for(0ms);
            }
        }

        future.get();

        // Create game controller
        auto* game = Chewman::Game::createInstance(std::bind(updateProgressBetween, 0.8f, 0.97f, std::placeholders::_1));
        if (!engine->getEngineSettings().particlesEnabled && game->getGraphicsManager().getSettings().particleEffects != Chewman::ParticlesSettings::None)
        {
            auto settings = game->getGraphicsManager().getSettings();
            settings.particleEffects = Chewman::ParticlesSettings::None;
            game->getGraphicsManager().setSettings(settings);
        }

        // Store all cache
        if (engine->getPipelineCacheManager()->isNew())
        {
            std::cout << "Storing pipeline cache." << std::endl;
            engine->getPipelineCacheManager()->store();
        }

        //auto windowSize = engine->getRenderWindowSize();

        if (game->getGraphicsManager().getSettings().effectSettings == Chewman::EffectSettings::High)
        {
            //windowSize.x = 1024; windowSize.y = 768;
            engine->getPostEffectManager()->addPostEffect("OnlyBrightEffect", "OnlyBrightEffect", resolution.x / 4, resolution.y / 4);
            engine->getPostEffectManager()->addPostEffect("VBlurEffect", "VBlurEffect", resolution.x / 8, resolution.y / 8);
            engine->getPostEffectManager()->addPostEffect("HBlurEffect", "HBlurEffect", resolution.x / 8, resolution.y / 8);
            engine->getPostEffectManager()->addPostEffect("BloomEffect", "BloomEffect", resolution.x, resolution.y);
        }

        // create camera
        camera->setNearFarPlane(0.1f, 100.0f);
        //engine->getSceneManager()->setSkybox("Skybox2");

        auto sunLight = engine->getSceneManager()->getLightManager()->getDirectionLight();
        if (graphicsManager.getSettings().effectSettings == Chewman::EffectSettings::Low) {
            sunLight->getLightSettings().ambientStrength = {0.2f, 0.2f, 0.2f, 1.0f};
            sunLight->getLightSettings().diffuseStrength = {1.0f, 1.0f, 1.0f, 1.0f};
            sunLight->getLightSettings().specularStrength = {0.5f, 0.5f, 0.5f, 1.0f};
            sunLight->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(80, 80, -80)));
            sunLight->getLightSettings().castShadows = true;

        } else {
            sunLight->getLightSettings().ambientStrength = {0.08f, 0.08f, 0.08f, 1.0f};
            sunLight->getLightSettings().diffuseStrength = {0.15f, 0.15f, 0.15f, 1.0f};
            sunLight->getLightSettings().specularStrength = {0.08f, 0.08f, 0.08f, 1.0f};
            sunLight->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(-20, 80, 80)));
            sunLight->getLightSettings().castShadows = false;
        }

        updateProgress(1.0f);
        SDL_Delay(100);
        loadingScreen->hide();

        auto startTime = std::chrono::high_resolution_clock::now();
        auto prevTime = std::chrono::duration<float, std::chrono::seconds::period>(
                std::chrono::high_resolution_clock::now() - startTime).count();
        bool isPaused = false;
        bool isMusicEnabled = game->getSoundsManager().isMusicEnabled();

        while (running) {
            auto curTime = std::chrono::duration<float, std::chrono::seconds::period>(
                    std::chrono::high_resolution_clock::now() - startTime).count();
            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                game->processInput(event);
                if (event.type == SDL_WINDOWEVENT)
                {
                    std::cout << "Window event type: " << (int)event.window.event << std::endl;
                    if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                    {
                        std::cout << "wait idle" << std::endl;
                        isMusicEnabled = game->getSoundsManager().isMusicEnabled();
                        game->getSoundsManager().setMusicEnabled(false);
                        engine->finishRendering();
                        sleep(1);
                        std::cout << "launching poll events thread" << std::endl;
                        auto future = std::async(std::launch::async, [&] {
                            while(!isPaused) {
                                SDL_PollEvent(&event);
                                //engine->finishRendering();
                            }
                        });
                        engine->onPause();
                        isPaused = true;
                        future.get();
                        LOG("game minimized");
                    }
                    if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                    {
                        auto future = std::async(std::launch::async, [&] {
                            while(isPaused) {
                                SDL_PollEvent(&event);
                                engine->finishRendering();
                            }
                        });
                        //std::async(std::launch::async, [&] {
                            engine->onResume();
                            isPaused = false;
                        //});
                        future.get();
                        game->getSoundsManager().setMusicEnabled(isMusicEnabled);
                        LOG("game restored");
                    }
                }
            }

            if (!isPaused)
            {
                game->update(curTime - prevTime);
                engine->renderFrame(curTime - prevTime);
            }

            prevTime = curTime;
        }

        //engine->destroyInstance();
    } catch (SVE::VulkanException& exception)
    {
        std::string message = std::string("Application error: ") + exception.what();
        LOG("Exception occured");
        std::cout << message << std::endl;

        if (exception.getVkResult() == VK_ERROR_SURFACE_LOST_KHR || exception.getVkResult() == VK_ERROR_DEVICE_LOST)
        {
            if (isMinimized)
            {
                Chewman::System::restartApp();
                return 0;
            } else {
                Chewman::System::exitApp();
                return 0;
            }
        }

        showAlert(message.c_str());
        std::exit(1);
    }
 
    LOG("finished");

    //SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}