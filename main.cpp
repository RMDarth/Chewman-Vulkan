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

#include "SVE/ShaderSettings.h"
#include "SVE/VulkanInstance.h"
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

SVE::MeshSettings calculateFrustum()
{
    const auto& camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
    const auto& cameraSettings = camera->getCameraSettings();

    SVE::UniformData data;
    camera->fillUniformData(data);

    auto cameraTransform = camera->getViewMatrix();

    // Pull camera basis
    glm::vec3 axis_x = glm::vec3(1,0,0);//cameraTransform[0];
    glm::vec3 axis_y = glm::vec3(0,1,0);//cameraTransform[1];
    glm::vec3 axis_z = glm::vec3(0,0,1);//cameraTransform[2];

    float zf = -50.0f;//-cameraSettings.farPlane;
    float zn = 0.0f;//-cameraSettings.nearPlane;// + 15.1f;

    // Near/far plane center points
    glm::vec3 near_center = axis_z * zn ;
    glm::vec3 far_center = axis_z * zf;

    // Get projected viewport extents on near/far planes
    float e = tanf(glm::radians(cameraSettings.fieldOfView) * 0.5f);
    float near_ext_y = e * zn;
    float near_ext_x = near_ext_y * cameraSettings.aspectRatio;
    float far_ext_y = e * zf;
    float far_ext_x = far_ext_y * cameraSettings.aspectRatio;

    std::vector<glm::vec3> points(8);
    // Points are just offset from the center points along camera basis
    points[0] = near_center - axis_x * near_ext_x - axis_y * near_ext_y;
    points[1] = near_center - axis_x * near_ext_x + axis_y * near_ext_y;
    points[2] = near_center + axis_x * near_ext_x + axis_y * near_ext_y;
    points[3] = near_center + axis_x * near_ext_x - axis_y * near_ext_y;
    points[4] = far_center  - axis_x * far_ext_x  - axis_y * far_ext_y;
    points[5] = far_center  - axis_x * far_ext_x  + axis_y * far_ext_y;
    points[6] = far_center  + axis_x * far_ext_x  + axis_y * far_ext_y;
    points[7] = far_center  + axis_x * far_ext_x  - axis_y * far_ext_y;

    //auto moveTransform = glm::translate(glm::mat4(1), glm::vec3(camera->getPosition()));

    for (auto& point : points)
    {
        point = camera->getTotalTransformation() * glm::vec4(point, 1.0f);
    }

    auto viewMatrix = glm::lookAt(glm::vec3(150.0f, 150.0f, -150.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 min(FLT_MAX,FLT_MAX,FLT_MAX,1);
    glm::vec4 max(FLT_MIN,FLT_MIN,FLT_MIN,1);

    for (auto& point : points)
    {
        auto newPoint = glm::mat3(viewMatrix) * point;

        min.x = std::min(min.x, newPoint.x);
        min.y = std::min(min.y, newPoint.y);
        min.z = std::min(min.z, newPoint.z);

        max.x = std::max(max.x, newPoint.x);
        max.y = std::max(max.y, newPoint.y);
        max.z = std::max(max.z, newPoint.z);
    }

    auto inverseView = glm::inverse(glm::mat3(viewMatrix));
    //min = viewMatrix * min;
    //max = viewMatrix * max;

    std::vector<glm::vec3> newPoints =
            {
                inverseView * glm::vec3(min.x, min.y, min.z),
                inverseView * glm::vec3(min.x, max.y, min.z),
                inverseView * glm::vec3(max.x, max.y, min.z),
                inverseView * glm::vec3(max.x, min.y, min.z),
                inverseView * glm::vec3(min.x, min.y, max.z),
                inverseView * glm::vec3(min.x, max.y, max.z),
                inverseView * glm::vec3(max.x, max.y, max.z),
                inverseView * glm::vec3(max.x, min.y, max.z)
            };

    std::vector<glm::vec2> texCoords =
            {
                    {0, 0},
                    {0, 1},
                    {1, 1},
                    {1, 1},
                    {1, 0},
                    {0, 0},
                    {0, 1},
                    {1, 1}
            };

    std::vector<uint32_t> indexes
            {
                0, 1, 2,
                0, 2, 3,

                //0, 2, 1,
                //0, 3, 2,

                0, 5, 1,
                0, 4, 5,

                3, 2, 6,
                3, 6, 7,

                4, 6, 5,
                4, 7, 6

                //4, 5, 6,
                //4, 6, 7,


            };
    std::vector<glm::vec3> normals(8, glm::vec3(0.2f, 1, 0.2f));
    std::vector<glm::vec3> colors(8, glm::vec3(1.0f, 1.0f, 1.0f));

    SVE::MeshSettings settings {};

    settings.name = "Frustum";
    settings.vertexPosData = std::move(newPoints);
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
    camera->setYawPitchRoll(glm::vec3(glm::radians(20.0f), glm::radians(-30.0f), 0.0f));
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

    // temp - frustum
    auto frustumMeshSettings = calculateFrustum();
    meshSettings.materialName = "Yellow";
    auto frustumMesh = std::make_shared<SVE::Mesh>(frustumMeshSettings);
    engine->getMeshManager()->registerMesh(frustumMesh);

    // create water mesh
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
    std::shared_ptr<SVE::Entity> bigFloorEntity = std::make_shared<SVE::MeshEntity>("Frustum");
    std::shared_ptr<SVE::MeshEntity> waterEntity = std::make_shared<SVE::MeshEntity>("WaterMesh");
    waterEntity->setMaterial("WaterReflection");
    waterEntity->setIsReflected(false);
    waterEntity->setCastShadows(false);
    //floorEntity->setMaterial("Floor");
    meshEntity->setMaterial("Yellow");
    meshEntity2->setMaterial("Blue");
    terrainEntity->setMaterial("Terrain");
    bigFloorEntity->setMaterial("TestMaterial");

    // configure and attach objects to nodes
    newNode->attachEntity(meshEntity);
    newNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(5, 0, 2)));

    {
        terrainNode->attachEntity(terrainEntity);
        auto nodeTransform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        nodeTransform = glm::translate(glm::mat4(1), glm::vec3(0, 15, 0)) * nodeTransform;
        terrainNode->setNodeTransformation(nodeTransform);
    }

    floorNode->attachEntity(floorEntity);
    floorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-20, 5, 0)));
    waterNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));
    waterNode->attachEntity(waterEntity);
    bigFloorNode->attachEntity(bigFloorEntity);
    //bigFloorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -5, 0)));

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
                    engine->getSceneManager()->getLight()->huita = !engine->getSceneManager()->getLight()->huita;
                    if (newNode2->getAttachedEntities().empty())
                    {

                        newNode2->attachEntity(meshEntity2);
                    } else {
                        newNode2->detachEntity(meshEntity2);
                    }
                }
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                if (event.motion.state && SDL_BUTTON(1))
                    moveCamera(event.motion, camera);
            }
        }

        newNode2->setNodeTransformation(glm::translate(glm::mat4(1), camera->getPosition()));
        auto frustumMeshSettings = calculateFrustum();
        frustumMesh->updateMesh(frustumMeshSettings);

        auto duration = std::chrono::duration<float, std::chrono::seconds::period>(curTime - prevTime).count();
        //std::cout << 1/duration << std::endl;

        const Uint8* keystates = SDL_GetKeyboardState(nullptr);
        moveCamera(keystates, duration, camera);
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