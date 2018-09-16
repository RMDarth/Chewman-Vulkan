#include <iostream>
#include "SDL2/SDL.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"
#include "SVE/LightNode.h"
#include "SVE/MeshEntity.h"
#include "SVE/ResourceManager.h"
#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <chrono>

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
    auto nodeTransformation = camera->getNodeTransformation();
    switch (keycode)
    {
        case SDLK_a:
            nodeTransformation = nodeTransformation * glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0, 1, 0));
            camera->setNodeTransformation(nodeTransformation);
            //camera->setNodeTransformation(glm::translate(camera->getNodeTransformation(), glm::vec3(-0.1, 0, 0)));
            break;
        case SDLK_d:
            nodeTransformation = nodeTransformation * glm::rotate(glm::mat4(1.0f), glm::radians(-2.0f), glm::vec3(0, 1, 0));
            camera->setNodeTransformation(nodeTransformation);
            //camera->setNodeTransformation(glm::translate(camera->getNodeTransformation(), glm::vec3(0.1, 0, 0)));
            break;
        case SDLK_w:
            camera->setNodeTransformation(glm::translate(camera->getNodeTransformation(), glm::vec3(0, 0.1, 0)));
            break;
        case SDLK_s:
            camera->setNodeTransformation(glm::translate(camera->getNodeTransformation(), glm::vec3(0, -0.1, 0)));
            break;
    }
}


int main(int argv, char** args)
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
    engine->getResourceManager()->loadFolder("resources");

    // configure light
    engine->getSceneManager()->getLight()->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(200, 200, -200)));

    // create camera
    auto camera = engine->getSceneManager()->createMainCamera();
    camera->setNearFarPlane(0.1f, 200.0f);
    camera->setLookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // create skybox
    engine->getSceneManager()->setSkybox("Skybox");

    // create nodes
    auto newNodeMid = engine->getSceneManager()->createSceneNode();
    auto newNode = engine->getSceneManager()->createSceneNode();
    auto newNode2 = engine->getSceneManager()->createSceneNode();
    engine->getSceneManager()->getRootNode()->attachSceneNode(newNodeMid);
    newNodeMid->attachSceneNode(newNode);
    engine->getSceneManager()->getRootNode()->attachSceneNode(newNode2);

    // create entities
    std::shared_ptr<SVE::Entity> meshEntity = std::make_shared<SVE::MeshEntity>("trashman");
    std::shared_ptr<SVE::Entity> meshEntity2 = std::make_shared<SVE::MeshEntity>("trashman");
    meshEntity->setMaterial("Yellow");
    meshEntity2->setMaterial("Blue");

    // configure and attach objects to nodes
    newNode->attachEntity(meshEntity);
    newNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(1, 0, 0)));

    bool quit = false;
    bool skiprendering = false;
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
                    if (!skiprendering)
                        engine->resizeWindow();
                    //    renderer->resizeWindow();
                }
                if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                {
                    skiprendering = true;
                    std::cout << "skip rendering" << std::endl;
                }
                if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                {
                    skiprendering = false;
                }
            }
            if (event.type == SDL_KEYUP)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    if (newNode2->getAttachedEntities().size() == 0)
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
                /*if (event.key.keysym.sym == SDLK_a)
                {
                    newNodeMid->setNodeTransformation(glm::translate(newNodeMid->getNodeTransformation(), glm::vec3(-1, 0, 0)));
                }*/
            }
        }
        auto duration = std::chrono::duration<float, std::chrono::seconds::period>(curTime - prevTime).count();
        //std::cout << 1/duration << std::endl;

        SDL_Delay(1);
        if (!skiprendering)
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