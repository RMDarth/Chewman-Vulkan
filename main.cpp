#include <iostream>
#include "SDL2/SDL.h"
#include "VulkanRenderer.h"
#include "SVE/Engine.h"
#include "SVE/Material.h"
#include "SVE/MeshEntity.h"
#include "SVE/MaterialManager.h"
#include "SVE/ShaderManager.h"
#include "SVE/SceneManager.h"
#include "SVE/MeshManager.h"
#include "SVE/CameraNode.h"
#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>


int main(int argv, char** args)
{
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            "Chewman",
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

    SVE::EngineSettings settings;
    settings.useValidation = true;
    SVE::Engine* engine = SVE::Engine::createInstance(window, settings);

    auto camera = engine->getSceneManager()->createMainCamera();
    camera->setNearFarPlane(0.1f, 1000.0f);
    camera->setLookAt(glm::vec3(200.0f, 200.0f, 200.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    SVE::ShaderSettings vertexShaderSettings {};
    vertexShaderSettings.name = "vertexShader";
    vertexShaderSettings.filename = "shaders/vert.spv";
    vertexShaderSettings.shaderType = SVE::ShaderType::VertexShader;
    vertexShaderSettings.uniformList.push_back({SVE::UniformType::ModelMatrix});
    vertexShaderSettings.uniformList.push_back({SVE::UniformType::ViewMatrix});
    vertexShaderSettings.uniformList.push_back({SVE::UniformType::ProjectionMatrix});

    std::shared_ptr<SVE::ShaderInfo> vertexShader = std::make_shared<SVE::ShaderInfo>(vertexShaderSettings);
    engine->getShaderManager()->registerShader(vertexShader);

    SVE::ShaderSettings fragmentShaderSettings {};
    fragmentShaderSettings.name = "fragmentShader";
    fragmentShaderSettings.filename = "shaders/frag.spv";
    fragmentShaderSettings.shaderType = SVE::ShaderType::FragmentShader;
    fragmentShaderSettings.samplerNamesList.emplace_back("texSampler");

    std::shared_ptr<SVE::ShaderInfo> fragmentShader = std::make_shared<SVE::ShaderInfo>(fragmentShaderSettings);
    engine->getShaderManager()->registerShader(fragmentShader);

    SVE::MaterialSettings materialSettings;
    materialSettings.name = "Material #2";
    materialSettings.vertexShaderName = "vertexShader";
    materialSettings.fragmentShaderName = "fragmentShader";
    materialSettings.textures.push_back({"texSampler", "textures/trashman_tex.png"});

    std::shared_ptr<SVE::Material> material = std::make_shared<SVE::Material>(materialSettings);
    engine->getMaterialManager()->registerMaterial(material);

    std::shared_ptr<SVE::Mesh> mesh = std::make_shared<SVE::Mesh>("trashman", "models/trashman.fbx");
    engine->getMeshManager()->registerMesh(mesh);

    std::shared_ptr<SVE::Entity> meshEntity = std::make_shared<SVE::MeshEntity>("trashman");
    engine->getSceneManager()->getRootNode()->attachEntity(meshEntity);

    auto newNode = engine->getSceneManager()->createSceneNode();
    newNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(50, 0, 0)));
    engine->getSceneManager()->getRootNode()->attachSceneNode(newNode);

    std::shared_ptr<SVE::Entity> meshEntity2 = std::make_shared<SVE::MeshEntity>("trashman");
    //newNode->attachEntity(meshEntity2);

    //std::shared_ptr<Renderer> renderer = std::make_shared<VulkanRenderer>(window, true);

    bool quit = false;
    bool skiprendering = false;
    while(!quit) {

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
                    //if (!skiprendering)
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
        }

        SDL_Delay(2);
        if (!skiprendering)
        {
            engine->renderFrame();
            //renderer->drawFrame();
        }
    }

    //renderer->finishRendering();
    //renderer.reset();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}