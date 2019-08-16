#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"
#include "SVE/MeshEntity.h"
#include "SVE/ResourceManager.h"
#include "SVE/LightManager.h"
#include "SVE/MeshManager.h"
#include "SVE/ParticleSystemManager.h"
#include "SVE/ParticleSystemEntity.h"

#include "Game/GameMap.h"

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <memory>
#include <mutex>


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

void moveLight(SDL_Keycode key, std::shared_ptr<SVE::LightNode>& lightNode)
{
    static float x = 50;
    static float y = 50;
    static float z = -50;
    const float speed = 5.0f;
    bool updated = false;

    if (key == SDLK_UP)
    {
        z = z + speed;
        updated = true;

    }
    if (key == SDLK_RIGHT)
    {
        x = x + speed;
        updated = true;
    }
    if (key == SDLK_DOWN)
    {
        z = z - speed;
        updated = true;

    }
    if (key == SDLK_LEFT)
    {
        x = x - speed;
        updated = true;
    }
    if (key == SDLK_n)
    {
        y -= speed;
        updated = true;
    }
    if (key == SDLK_m)
    {
        y += speed;
        updated = true;
    }

    if (updated)
        lightNode->setNodeTransformation(
                glm::translate(glm::mat4(1), glm::vec3(x, y, z)));
}

void configFloor(SDL_Keycode key, std::shared_ptr<SVE::SceneNode>& floor)
{
    static float x = -12;
    static float z = -21;
    static float y = -11;
    bool updated = false;
    /*if (key == SDLK_UP)
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
    }*/
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

glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
    start = normalize(start);
    dest = normalize(dest);

    float cosTheta = dot(start, dest);
    glm::vec3 rotationAxis;

    rotationAxis = cross(start, dest);

    float s = sqrt( (1+cosTheta)*2 );
    float invs = 1 / s;

    return glm::quat(
            s * 0.5f,
            rotationAxis.x * invs,
            rotationAxis.y * invs,
            rotationAxis.z * invs
    );

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
        auto mat = glm::toMat4(rotationBetweenVectors(glm::vec3(0.0, 1.0, 0.0), normal));
        mat = mat * glm::scale(glm::mat4(1), glm::vec3(width, 0, height));
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


auto CreateTeleport(SVE::Engine* engine, glm::vec3 position, glm::vec3 color)
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [engine]{
        auto meshSettings = constructPlane("TeleportBase",
                                           glm::vec3(0, 0, 0), 0.9f, 0.9f, glm::vec3(0.0f, 1.0f, 0.0f));
        meshSettings.materialName = "TeleportBaseMaterial";
        auto teleportBaseMesh = std::make_shared<SVE::Mesh>(meshSettings);
        engine->getMeshManager()->registerMesh(teleportBaseMesh);
    });

    auto adjustColor = [](float comp)
    {
        return comp < 0.01f ? 0.4f : comp;
    };

    auto teleportNode = engine->getSceneManager()->createSceneNode();
    teleportNode->setNodeTransformation(glm::translate(glm::mat4(1), position));
    engine->getSceneManager()->getRootNode()->attachSceneNode(teleportNode);
    std::shared_ptr<SVE::ParticleSystemEntity> teleportPS = std::make_shared<SVE::ParticleSystemEntity>("TeleportStars");
    teleportPS->getMaterialInfo()->diffuse = glm::vec4(color, 1.0f);
    teleportNode->attachEntity(teleportPS);

    auto teleportBaseNode = engine->getSceneManager()->createSceneNode();
    auto teleportCircleNode = engine->getSceneManager()->createSceneNode();
    auto teleportLightNode = engine->getSceneManager()->createSceneNode();
    auto teleportPlatformNode = engine->getSceneManager()->createSceneNode();
    teleportNode->attachSceneNode(teleportBaseNode);
    teleportNode->attachSceneNode(teleportCircleNode);
    teleportNode->attachSceneNode(teleportLightNode);
    teleportNode->attachSceneNode(teleportPlatformNode);
    {
        teleportPlatformNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-1.0, -0.2, 1.0)));
        std::shared_ptr<SVE::MeshEntity> teleportPlatformEntity = std::make_shared<SVE::MeshEntity>("teleport");
        teleportPlatformEntity->setMaterial("TeleportPlatformMaterial");
        teleportPlatformEntity->getMaterialInfo()->diffuse = { adjustColor(color.r), adjustColor(color.r), adjustColor(color.b), 1.0f };
        teleportPlatformNode->attachEntity(teleportPlatformEntity);

        std::shared_ptr<SVE::MeshEntity> teleportBaseEntity = std::make_shared<SVE::MeshEntity>("TeleportBase");
        teleportBaseEntity->setRenderLast();
        teleportBaseEntity->setCastShadows(false);
        //teleportBaseEntity->getMaterialInfo()->diffuse = { 1.0f, 0.0f, 0.0f, 1.0f };
        teleportBaseNode->attachEntity(teleportBaseEntity);

        std::shared_ptr<SVE::MeshEntity> teleportCircleEntity = std::make_shared<SVE::MeshEntity>("cylinder");
        teleportCircleEntity->setMaterial("TeleportCircleMaterial");
        teleportCircleEntity->setRenderLast();
        teleportCircleEntity->setCastShadows(false);
        //teleportCircleEntity->getMaterialInfo()->diffuse = { 1.0f, 0.0f, 0.0f, 1.0f };
        teleportCircleNode->attachEntity(teleportCircleEntity);

        // Add light effect
        teleportLightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 1.0, 0)));
        SVE::LightSettings lightSettings {};
        lightSettings.lightType = SVE::LightType::LineLight;
        lightSettings.secondPoint = position + glm::vec3(0, -0.7, 0.0);
        lightSettings.castShadows = false;
        lightSettings.diffuseStrength = glm::vec4(color, 1.0f);
        lightSettings.specularStrength = glm::vec4(color * 0.5f, 1.0f);
        lightSettings.ambientStrength = { color * 0.2f, 1.0f };
        lightSettings.shininess = 16;
        lightSettings.constAtten = 1.0f * 1.8f;
        lightSettings.linearAtten = 0.35f * 0.05f;
        lightSettings.quadAtten = 0.44f * 0.05f;
        auto lightManager = engine->getSceneManager()->getLightManager();
        auto lightNode = std::make_shared<SVE::LightNode>(lightSettings, lightManager->getLightCount());
        lightManager->setLight(lightNode, lightManager->getLightCount());
        teleportLightNode->attachSceneNode(lightNode);
    }

    return std::make_tuple(teleportBaseNode, teleportCircleNode);
}

int runGame()
{
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    window = SDL_CreateWindow(
            "Chewman Vulkan",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1024, 768,
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
        auto sunLight = engine->getSceneManager()->getLightManager()->getDirectionLight();
        sunLight->setNodeTransformation(
                glm::translate(glm::mat4(1), glm::vec3(50, 50, -50)));
        if (engine->getSceneManager()->getLightManager()->getLightCount() >= 1)
            engine->getSceneManager()->getLightManager()->getLight(0)->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(25, 5, -5)));
        if (engine->getSceneManager()->getLightManager()->getLightCount() >= 2)
            engine->getSceneManager()->getLightManager()->getLight(1)->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(-5, 10, 5)));


        auto particleNode = engine->getSceneManager()->createSceneNode();
        particleNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(3, 5, 0)));
        engine->getSceneManager()->getRootNode()->attachSceneNode(particleNode);
        std::shared_ptr<SVE::ParticleSystemEntity> particleSystem = std::make_shared<SVE::ParticleSystemEntity>("FireParticle");
        //particleNode->attachEntity(particleSystem);

        engine->getSceneManager()->getLightManager()->removeLight(0);
        engine->getSceneManager()->getLightManager()->removeLight(1);

        // create camera
        auto camera = engine->getSceneManager()->createMainCamera();
        camera->setNearFarPlane(0.1f, 500.0f);
        camera->setPosition(glm::vec3(5.0f, 5.0f, 5.0f));

        // create skybox
        engine->getSceneManager()->setSkybox("Skybox2");

        // create floor
        auto meshSettings = constructPlane("Floor", glm::vec3(0, 0, 0), 10.0f, 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        meshSettings.materialName = "Floor";
        auto floorMesh = std::make_shared<SVE::Mesh>(meshSettings);
        engine->getMeshManager()->registerMesh(floorMesh);

        // create water mesh
        auto waterMeshSettings = constructPlane("WaterMesh", glm::vec3(0, -0.5f, 0), 100.0f, 100.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        waterMeshSettings.materialName = "WaterReflection";
        auto waterMesh = std::make_shared<SVE::Mesh>(waterMeshSettings);
        engine->getMeshManager()->registerMesh(waterMesh);

        auto bigFloorMeshSettings = constructPlane("BigFloor", glm::vec3(0, 0, 0), 30.0f, 30.0f,
                                                   glm::vec3(0.0f, 1.0f, 0.0f));
        bigFloorMeshSettings.materialName = "Floor";
        auto bigFloorMesh = std::make_shared<SVE::Mesh>(bigFloorMeshSettings);
        engine->getMeshManager()->registerMesh(bigFloorMesh);

        // Create level floor
        Chewman::GameMap gameMap;
        gameMap.LoadMap("resources/game/levels/level1.map");

        // create teleport test
        std::array<std::shared_ptr<SVE::SceneNode>, 4> baseNodes;
        std::array<std::shared_ptr<SVE::SceneNode>, 4> circleNodes;
        std::tie(baseNodes[0], circleNodes[0]) = CreateTeleport(engine, glm::vec3(9, 0.2, -3), glm::vec3(1.0, 0.0, 0.0));
        std::tie(baseNodes[1], circleNodes[1]) = CreateTeleport(engine, glm::vec3(18, 0.2, -3), glm::vec3(0.0, 1.0, 0.0));
        std::tie(baseNodes[2], circleNodes[2]) = CreateTeleport(engine, glm::vec3(18, 0.2, -18), glm::vec3(1.0, 0.0, 1.0));
        std::tie(baseNodes[3], circleNodes[3]) = CreateTeleport(engine, glm::vec3(3, 0.2, -9), glm::vec3(0.0, 1.0, 1.0));

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
        waterEntity->setCastShadows(true);
        meshEntity->setMaterial("Yellow");
        meshEntity2->setMaterial("Blue");
        terrainEntity->setMaterial("Terrain");
        //bigFloorEntity->setMaterial("TestMaterial");

        // configure and attach objects to nodes
        newNode->attachEntity(meshEntity);
        newNodeMid->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(3, 0, -3)));
        newNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(5, 0, 2)));

        {
            terrainNode->attachEntity(terrainEntity);
            auto nodeTransform = glm::translate(glm::mat4(1), glm::vec3(-12, -31, -21));
            terrainNode->setNodeTransformation(nodeTransform);
        }

        floorNode->attachEntity(floorEntity);
        floorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(-20, 5, 0)));
        waterNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));
        waterNode->attachEntity(waterEntity);
        //bigFloorNode->attachEntity(bigFloorEntity);
        //bigFloorNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, -5, 0)));
        //levelNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(10, 5, 0)));
        //levelNode->attachEntity(levelEntity);

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
                            auto& emitter = particleSystem->getSettings().particleEmitter;
                            auto& affector = particleSystem->getSettings().particleAffector;
                            emitter.minLife = 0.0f;
                            emitter.maxLife = 0.0f;

                            affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, -3.5f);
                            affector.lifeDrain = 5.0f;

                        }
                        else
                        {
                            auto& emitter = particleSystem->getSettings().particleEmitter;
                            auto& affector = particleSystem->getSettings().particleAffector;
                            emitter.minLife = 10.0f;
                            emitter.maxLife = 10.0f;

                            affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, 0.0f);
                            affector.lifeDrain = 0.0f;

                            newNode2->detachEntity(meshEntity2);
                        }
                    }

                    configFloor(event.key.keysym.sym, terrainNode);
                    moveLight(event.key.keysym.sym, sunLight);
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
                for (auto& baseNode : baseNodes)
                {
                    updateNode(baseNode, curTime * 2);
                }
                for (auto& circleNode : circleNodes)
                {
                    updateNode(circleNode, curTime * 5);
                }
                gameMap.Update(curTime - prevTime);
            }


            // Temp : update fireline
            /*auto& emitter = particleSystem->getSettings().particleEmitter;
            auto& affector = particleSystem->getSettings().particleAffector;
            static float currentLife = 0.0f;
            if (currentLife > 0)
            {
                emitter.minLife = std::max(currentLife, 10.0f);
                emitter.maxLife = std::max(currentLife, 10.0f);

                affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, 0.0f);
                affector.lifeDrain = 0.0f;
            } else {
                emitter.minLife = 0;
                emitter.maxLife = 0;

                affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, -3.5f);
                affector.lifeDrain = 5.0f;
            }
            emitter.minSpeed = 5;
            emitter.maxSpeed = 5;
            emitter.emissionRate = 40;

            currentLife += (curTime - prevTime);
            if (currentLife > 10.0f)
                currentLife = -10.0f;*/

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
    //try
    {
        return runGame();
    }
    //catch (const std::exception& ex)
    {
    //    std::cerr << "Unhandled exception: " << ex.what() << std::endl;
     //   throw;
        //return 2;
    }
}