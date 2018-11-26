#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"
#include "SVE/MeshEntity.h"
#include "SVE/ResourceManager.h"
#include "SVE/LightManager.h"
#include "SVE/MeshManager.h"

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <memory>

// Thanks to:
// Karl "ThinMatrix" for his video blogs on OpenGL techniques
// Niko Kauppi for his Vulkan video tutorials
// Alexander Overvoorde for his Vulkan tutorial website (https://vulkan-tutorial.com)
// Sascha Willems for his Vulkan examples git repository
// Joey de Vries for his OpenGL tutorials (learnopengl.com)
// Eray Meiri for his OGL dev tutorials (ogldev.org)
// Pawel Lapinski for his Vulkan Cookbook and compute shaders receipts

void updateNode(std::shared_ptr<SVE::SceneNode>& node, float time)
{
    auto nodeTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    node->setNodeTransformation(nodeTransform);
}

void moveCamera(const Uint8* keystates, float deltaTime, std::shared_ptr<SVE::CameraNode>& camera)
{
    auto pos = camera->getPosition();
    auto yawPitchRoll = camera->getYawPitchRoll();

    if (keystates[SDL_SCANCODE_A])
        camera->movePosition(glm::vec3(-12.0f*deltaTime,0,0));
    if (keystates[SDL_SCANCODE_D])
        camera->movePosition(glm::vec3(12.0f*deltaTime,0,0));
    if (keystates[SDL_SCANCODE_W])
        camera->movePosition(glm::vec3(0,0,-12.0f*deltaTime));
    if (keystates[SDL_SCANCODE_S])
        camera->movePosition(glm::vec3(0,0,12.0f*deltaTime));

    //camera->movePosition(glm::vec3(0,0,5.0f*deltaTime));
}

void configFloor(SDL_Keycode key, std::shared_ptr<SVE::SceneNode>& floor)
{
    static float x = -12;
    static float z = -21;
    static float y = -11;
    bool updated = false;
    if (key == SDLK_UP)
    {
        z = z + 1;
        updated = true;

    }
    if (key == SDLK_RIGHT)
    {
        x = x + 1;
        updated = true;
    }
    if (key == SDLK_DOWN)
    {
        z = z - 1;
        updated = true;

    }
    if (key == SDLK_LEFT)
    {
        x = x - 1;
        updated = true;
    }
    if (key == SDLK_z)
    {
        y = y + 1;
        updated = true;

    }
    if (key == SDLK_x)
    {
        y = y - 1;
        updated = true;
    }

    if (updated)
    {
        auto nodeTransform = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
        floor->setNodeTransformation(nodeTransform);

        std::cout << x << " " << y << " " << z << std::endl;
    }
}

void rotateCamera(SDL_MouseMotionEvent& event, std::shared_ptr<SVE::CameraNode>& camera)
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
    {

        // load resources
        engine->getResourceManager()->loadFolder("resources/shaders");
        engine->getResourceManager()->loadFolder("resources/materials");
        engine->getResourceManager()->loadFolder("resources/models");
        engine->getResourceManager()->loadFolder("resources");

        // configure light
        engine->getSceneManager()->getLightManager()->getDirectionLight()->setNodeTransformation(
                glm::translate(glm::mat4(1), glm::vec3(15, 15, -15)));
        if (engine->getSceneManager()->getLightManager()->getLightCount() >= 1)
            engine->getSceneManager()->getLightManager()->getLight(0)->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(5, 5, 5)));
        if (engine->getSceneManager()->getLightManager()->getLightCount() >= 2)
            engine->getSceneManager()->getLightManager()->getLight(1)->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(-5, 5, 5)));

        // create camera
        auto camera = engine->getSceneManager()->createMainCamera();
        camera->setNearFarPlane(0.1f, 500.0f);
        camera->setPosition(glm::vec3(5.0f, 5.0f, 5.0f));
        //camera->setYawPitchRoll(glm::vec3(glm::radians(20.0f), glm::radians(-30.0f), 0.0f));
        //camera->setLookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // create shadowmap
        //engine->getSceneManager()->enableShadowMap();

        // init water
        //engine->getSceneManager()->getWater()->setHeight(0.0f);

        // create skybox
        engine->getSceneManager()->setSkybox("Skybox2");

        // create floor
        auto meshSettings = constructPlane("Floor", glm::vec3(0, 0, 0), 10.0f, 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        meshSettings.materialName = "Floor";
        auto floorMesh = std::make_shared<SVE::Mesh>(meshSettings);
        engine->getMeshManager()->registerMesh(floorMesh);

        // create water mesh
        meshSettings.name = "WaterMesh";
        meshSettings.materialName = "WaterReflection";
        auto waterMesh = std::make_shared<SVE::Mesh>(meshSettings);
        engine->getMeshManager()->registerMesh(waterMesh);

        auto bigFloorMeshSettings = constructPlane("BigFloor", glm::vec3(0, 0, 0), 30.0f, 30.0f,
                                                   glm::vec3(0.0f, 1.0f, 0.0f));
        bigFloorMeshSettings.materialName = "Floor";
        auto bigFloorMesh = std::make_shared<SVE::Mesh>(bigFloorMeshSettings);
        engine->getMeshManager()->registerMesh(bigFloorMesh);

        // create nodes
        auto newNodeMid = engine->getSceneManager()->createSceneNode();
        auto newNode = engine->getSceneManager()->createSceneNode();
        auto newNode2 = engine->getSceneManager()->createSceneNode();
        auto terrainNode = engine->getSceneManager()->createSceneNode();
        auto floorNode = engine->getSceneManager()->createSceneNode();
        auto bigFloorNode = engine->getSceneManager()->createSceneNode();
        auto waterNode = engine->getSceneManager()->createSceneNode();
        engine->getSceneManager()->getRootNode()->attachSceneNode(newNodeMid);
        newNodeMid->attachSceneNode(newNode);
        engine->getSceneManager()->getRootNode()->attachSceneNode(newNode2);
        engine->getSceneManager()->getRootNode()->attachSceneNode(terrainNode);
        engine->getSceneManager()->getRootNode()->attachSceneNode(floorNode);
        engine->getSceneManager()->getRootNode()->attachSceneNode(waterNode);
        //engine->getSceneManager()->getRootNode()->attachSceneNode(bigFloorNode);

        // create entities
        std::shared_ptr<SVE::Entity> meshEntity = std::make_shared<SVE::MeshEntity>("trashman");
        std::shared_ptr<SVE::Entity> meshEntity2 = std::make_shared<SVE::MeshEntity>("trashman");
        std::shared_ptr<SVE::Entity> terrainEntity = std::make_shared<SVE::MeshEntity>("terrain");
        std::shared_ptr<SVE::Entity> floorEntity = std::make_shared<SVE::MeshEntity>("Floor");
        std::shared_ptr<SVE::MeshEntity> waterEntity = std::make_shared<SVE::MeshEntity>("WaterMesh");
        waterEntity->setMaterial("WaterReflection");
        waterEntity->setIsReflected(false);
        waterEntity->setCastShadows(false);
        meshEntity->setMaterial("Yellow");
        meshEntity2->setMaterial("Blue");
        terrainEntity->setMaterial("Terrain");
        //bigFloorEntity->setMaterial("TestMaterial");

        // configure and attach objects to nodes
        newNode->attachEntity(meshEntity);
        newNodeMid->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(1, 0, 1)));
        newNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(5, 0, 2)));

        {
            terrainNode->attachEntity(terrainEntity);
            auto nodeTransform = glm::translate(glm::mat4(1), glm::vec3(-12, -11, -21));
            terrainNode->setNodeTransformation(nodeTransform);
        }

        floorNode->attachEntity(floorEntity);
        floorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-20, 5, 0)));
        waterNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));
        waterNode->attachEntity(waterEntity);
        //bigFloorNode->attachEntity(bigFloorEntity);
        //bigFloorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -5, 0)));

        bool quit = false;
        bool skipRendering = false;
        auto prevTime = engine->getTime();
        while (!quit)
        {
            auto curTime = engine->getTime();
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
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
                        }
                        else
                        {
                            newNode2->detachEntity(meshEntity2);
                        }
                    }

                    configFloor(event.key.keysym.sym, terrainNode);
                }
                if (event.type == SDL_MOUSEMOTION)
                {
                    if (event.motion.state && SDL_BUTTON(1))
                        rotateCamera(event.motion, camera);
                }
            }

            const Uint8* keystates = SDL_GetKeyboardState(nullptr);
            moveCamera(keystates, curTime - prevTime, camera);
            SDL_Delay(1);
            if (!skipRendering)
            {
                engine->renderFrame();

                updateNode(newNode, curTime);
            }
            prevTime = curTime;
        }

        engine->finishRendering();

        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    engine->destroyInstance();

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