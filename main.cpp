#include <iostream>
#include "SDL2/SDL.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"
#include "SVE/LightNode.h"
#include "SVE/MeshEntity.h"
#include "SVE/ResourceManager.h"
#include "SVE/MeshManager.h"
#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <memory>
#include <chrono>

// Thanks to:
// Karl "ThinMatrix" for his video blogs on OpenGL techniques
// Niko Kauppi for his Vulkan video tutorials
// Alexander Overvoorde for his Vulkan tutorial website (https://vulkan-tutorial.com)
// Sascha Willems for his Vulkan examples git repository

void updateNode(std::shared_ptr<SVE::SceneNode>& node)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    auto nodeTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    node->setNodeTransformation(nodeTransform);
}

void moveCamera(SDL_Keycode keycode, std::shared_ptr<SVE::CameraNode>& camera)
{
    auto pos = camera->getPosition();
    auto yawPitchRoll = camera->getYawPitchRoll();
    switch (keycode)
    {
        case SDLK_a:
            camera->movePosition(glm::vec3(-0.15f,0,0));
            break;
        case SDLK_d:
            camera->movePosition(glm::vec3(0.15f,0,0));
            break;
        case SDLK_w:
            camera->movePosition(glm::vec3(0,0,-0.15f));
            break;
        case SDLK_s:
            camera->movePosition(glm::vec3(0,0,0.15f));
            break;
    }
}

void moveCamera(SDL_MouseMotionEvent& event, std::shared_ptr<SVE::CameraNode>& camera)
{
    auto yawPitchRoll = camera->getYawPitchRoll();
    yawPitchRoll.x += glm::radians(-event.xrel * 0.4f);
    yawPitchRoll.y += glm::radians(-event.yrel * 0.4f);
    camera->setYawPitchRoll(yawPitchRoll);
}

SVE::MeshSettings constructPlane(std::string name, glm::vec3 center, float width, float height, glm::vec3 normal)
{
    std::vector<glm::vec3> points =
            {
                    {-1.0f,  0.0f,  1.0f},
                    {-1.0f,  0.0f, -1.0f},
                    {1.0f,   0.0f, -1.0f},
                    {1.0f,   0.0f, -1.0f},
                    {1.0f,   0.0f, 1.0f},
                    {-1.0f,  0.0f, 1.0f},
            };
    std::vector<glm::vec2> texCoords =
            {
                    {0, 0},
                    {0, 1},
                    {1, 1},
                    {1, 1},
                    {1, 0},
                    {0, 0}
            };
    std::vector<uint32_t> indexes;
    std::vector<glm::vec3> normals(6, normal);
    std::vector<glm::vec3> colors(6, glm::vec3(1.0f, 1.0f, 1.0f));

    uint32_t currentIndex = 0;
    for (auto& point : points)
    {
        //auto mat = glm::orientation(glm::vec3(0.0f, 0.0f, 1.0f), normal);
        auto mat = glm::mat4(1) * glm::scale(glm::mat4(1), glm::vec3(width, 0, height));
        point = glm::vec3(mat * glm::vec4(point, 1.0f));
        point += center;

        indexes.push_back(currentIndex++);
    }

    SVE::MeshSettings settings {};

    settings.name = std::move(name);
    settings.vertexPosData = std::move(points);
    settings.vertexTexData = std::move(texCoords);
    settings.vertexNormalData = std::move(normals);
    settings.vertexColorData = std::move(colors);
    settings.indexData = std::move(indexes);
    settings.boneNum = 0;

    return settings;
}

int runGame()
{
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            "Chewman Vulkan",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800,
            600,
            SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if(!window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SVE::Engine* engine = SVE::Engine::createInstance(window, "resources/main.engine");

    // load resources
    engine->getResourceManager()->loadFolder("resources/shaders");
    engine->getResourceManager()->loadFolder("resources/materials");
    engine->getResourceManager()->loadFolder("resources/models");
    engine->getResourceManager()->loadFolder("resources");

    // configure light
    engine->getSceneManager()->getLight()->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(15, 15, -15)));

    // create camera
    auto camera = engine->getSceneManager()->createMainCamera();
    camera->setNearFarPlane(0.1f, 500.0f);
    camera->setPosition(glm::vec3(5.0f, 5.0f, 5.0f));
    camera->setYawPitchRoll(glm::vec3(0.0f, glm::degrees(-30.0f), 0.0f));
    //camera->setLookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // create shadowmap
    engine->getSceneManager()->enableShadowMap();

    // init water
    //engine->getSceneManager()->getWater()->setHeight(0.0f);

    // create skybox
    engine->getSceneManager()->setSkybox("Skybox");

    // create floor
    auto meshSettings = constructPlane("Floor", glm::vec3(0, 0, 0), 10.0f, 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    meshSettings.materialName = "Floor";
    auto floorMesh = std::make_shared<SVE::Mesh>(meshSettings);
    engine->getMeshManager()->registerMesh(floorMesh);

    meshSettings.name = "WaterMesh";
    meshSettings.materialName = "WaterReflection";
    auto waterMesh = std::make_shared<SVE::Mesh>(meshSettings);
    engine->getMeshManager()->registerMesh(waterMesh);

    auto bigFloorMeshSettings = constructPlane("BigFloor", glm::vec3(0, 0, 0), 30.0f, 30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    bigFloorMeshSettings.materialName = "Floor";
    auto bigFloorMesh = std::make_shared<SVE::Mesh>(bigFloorMeshSettings);
    engine->getMeshManager()->registerMesh(bigFloorMesh);

    // create nodes
    auto newNodeMid = engine->getSceneManager()->createSceneNode();
    auto newNode = engine->getSceneManager()->createSceneNode();
    auto newNode2 = engine->getSceneManager()->createSceneNode();
    auto newNode3 = engine->getSceneManager()->createSceneNode();
    auto floorNode = engine->getSceneManager()->createSceneNode();
    auto bigFloorNode = engine->getSceneManager()->createSceneNode();
    auto waterNode = engine->getSceneManager()->createSceneNode();
    engine->getSceneManager()->getRootNode()->attachSceneNode(newNodeMid);
    newNodeMid->attachSceneNode(newNode);
    engine->getSceneManager()->getRootNode()->attachSceneNode(newNode2);
    engine->getSceneManager()->getRootNode()->attachSceneNode(newNode3);
    engine->getSceneManager()->getRootNode()->attachSceneNode(floorNode);
    engine->getSceneManager()->getRootNode()->attachSceneNode(waterNode);
    engine->getSceneManager()->getRootNode()->attachSceneNode(bigFloorNode);

    // create entities
    std::shared_ptr<SVE::Entity> meshEntity = std::make_shared<SVE::MeshEntity>("trashman");
    std::shared_ptr<SVE::Entity> meshEntity2 = std::make_shared<SVE::MeshEntity>("trashman");
    std::shared_ptr<SVE::Entity> meshEntity3 = std::make_shared<SVE::MeshEntity>("terrain");
    std::shared_ptr<SVE::Entity> floorEntity = std::make_shared<SVE::MeshEntity>("Floor");
    std::shared_ptr<SVE::Entity> bigFloorEntity = std::make_shared<SVE::MeshEntity>("BigFloor");
    std::shared_ptr<SVE::Entity> waterEntity = std::make_shared<SVE::MeshEntity>("WaterMesh");
    waterEntity->setMaterial("WaterReflection");
    //floorEntity->setMaterial("Floor");
    meshEntity->setMaterial("Yellow");
    meshEntity2->setMaterial("Blue");
    meshEntity3->setMaterial("Terrain");

    // configure and attach objects to nodes
    newNode->attachEntity(meshEntity);
    newNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-9, 0, 0)));

    {
        newNode3->attachEntity(meshEntity3);
        auto nodeTransform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        nodeTransform = glm::translate(glm::mat4(1), glm::vec3(0, 15, 0)) * nodeTransform;
        newNode3->setNodeTransformation(nodeTransform);
    }

    floorNode->attachEntity(floorEntity);
    floorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-20, 0, 0)));
    waterNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));
    waterNode->attachEntity(waterEntity);
    //bigFloorNode->attachEntity(bigFloorEntity);
    bigFloorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -5, 0)));

    bool quit = false;
    bool skipRendering = false;
    auto prevTime = std::chrono::high_resolution_clock::now();
    decltype(prevTime) curTime;
    while(!quit) {
        curTime = std::chrono::high_resolution_clock::now();
        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            if(event.type == SDL_QUIT)
            {
                quit = true;
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    if (!skipRendering)
                        engine->resizeWindow();
                }
                if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                {
                    skipRendering = true;
                    std::cout << "skip rendering" << std::endl;
                }
                if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                {
                    skipRendering = false;
                }
            }
            if (event.type == SDL_KEYUP)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    if (newNode2->getAttachedEntities().empty())
                    {
                        newNode2->attachEntity(meshEntity2);
                    } else {
                        newNode2->detachEntity(meshEntity2);
                    }
                }
            }
            if (event.type == SDL_KEYDOWN)
            {
                moveCamera(event.key.keysym.sym, camera);
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                if (event.motion.state && SDL_BUTTON(1))
                    moveCamera(event.motion, camera);
            }
        }
        auto duration = std::chrono::duration<float, std::chrono::seconds::period>(curTime - prevTime).count();
        //std::cout << 1/duration << std::endl;

        SDL_Delay(1);
        if (!skipRendering)
        {
            engine->renderFrame();

            updateNode(newNode);
        }
        prevTime = curTime;
    }

    engine->finishRendering();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int main(int argv, char** args)
{
    try
    {
        return runGame();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        throw;
        //return 2;
    }
}