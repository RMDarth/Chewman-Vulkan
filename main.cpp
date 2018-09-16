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
#include "SVE/LightNode.h"
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
    //nodeTransform = glm::rotate(nodeTransform, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
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

    SVE::EngineSettings settings;
    settings.useValidation = true;
    SVE::Engine* engine = SVE::Engine::createInstance(window, settings);

    // create camera
    auto camera = engine->getSceneManager()->createMainCamera();
    camera->setNearFarPlane(0.1f, 200.0f);
    camera->setLookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // create light
    SVE::LightSettings lightSettings {};
    lightSettings.diffuseStrength = 1.0f;
    lightSettings.specularStrength = 0.5f;
    lightSettings.ambientStrength = 0.0f;
    lightSettings.shininess = 16;
    lightSettings.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    auto light = engine->getSceneManager()->createLight(lightSettings);
    light->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(200, 200, -200)));

    // Create shaders
    {
        SVE::ShaderSettings vertexShaderSettings{};
        vertexShaderSettings.name = "vertexShader";
        vertexShaderSettings.filename = "shaders/vert.spv";
        vertexShaderSettings.shaderType = SVE::ShaderType::VertexShader;
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::ModelMatrix});
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::ViewMatrix});
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::ProjectionMatrix});
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::BoneMatrices});
        vertexShaderSettings.maxBonesSize = 64;

        std::shared_ptr<SVE::ShaderInfo> vertexShader = std::make_shared<SVE::ShaderInfo>(vertexShaderSettings);
        engine->getShaderManager()->registerShader(vertexShader);

        SVE::ShaderSettings fragmentShaderSettings{};
        fragmentShaderSettings.name = "fragmentShader";
        fragmentShaderSettings.filename = "shaders/frag.spv";
        fragmentShaderSettings.shaderType = SVE::ShaderType::FragmentShader;
        fragmentShaderSettings.samplerNamesList.emplace_back("texSampler");
        // set light uniforms
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::LightPosition});
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::LightColor});
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::CameraPosition});
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::LightAmbient});
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::LightDiffuse});
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::LightSpecular});
        fragmentShaderSettings.uniformList.push_back({SVE::UniformType::LightShininess});

        std::shared_ptr<SVE::ShaderInfo> fragmentShader = std::make_shared<SVE::ShaderInfo>(fragmentShaderSettings);
        engine->getShaderManager()->registerShader(fragmentShader);
    }

    // Create skybox shaders
    {
        SVE::ShaderSettings vertexShaderSettings{};
        vertexShaderSettings.name = "skyboxVertexShader";
        vertexShaderSettings.filename = "shaders/skybox.vert.spv";
        vertexShaderSettings.shaderType = SVE::ShaderType::VertexShader;
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::ModelMatrix});
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::ViewMatrix});
        vertexShaderSettings.uniformList.push_back({SVE::UniformType::ProjectionMatrix});

        std::shared_ptr<SVE::ShaderInfo> vertexShader = std::make_shared<SVE::ShaderInfo>(vertexShaderSettings);
        engine->getShaderManager()->registerShader(vertexShader);

        SVE::ShaderSettings fragmentShaderSettings{};
        fragmentShaderSettings.name = "skyboxFragmentShader";
        fragmentShaderSettings.filename = "shaders/skybox.frag.spv";
        fragmentShaderSettings.shaderType = SVE::ShaderType::FragmentShader;
        fragmentShaderSettings.samplerNamesList.emplace_back("samplerCubeMap");

        std::shared_ptr<SVE::ShaderInfo> fragmentShader = std::make_shared<SVE::ShaderInfo>(fragmentShaderSettings);
        engine->getShaderManager()->registerShader(fragmentShader);
    }

    // Create materials
    {
        SVE::MaterialSettings materialSettings;
        materialSettings.name = "Yellow";
        materialSettings.vertexShaderName = "vertexShader";
        materialSettings.fragmentShaderName = "fragmentShader";
        materialSettings.textures.push_back({"texSampler", "textures/trashman_tex.png"});
        //materialSettings.invertCullFace = true;
        std::shared_ptr<SVE::Material> material = std::make_shared<SVE::Material>(materialSettings);
        engine->getMaterialManager()->registerMaterial(material);
    }
    {
        SVE::MaterialSettings materialSettings;
        materialSettings.name = "Blue";
        materialSettings.vertexShaderName = "vertexShader";
        materialSettings.fragmentShaderName = "fragmentShader";
        materialSettings.textures.push_back({"texSampler", "textures/trashantiman_tex.png"});

        std::shared_ptr<SVE::Material> material = std::make_shared<SVE::Material>(materialSettings);
        engine->getMaterialManager()->registerMaterial(material);
    }
    {
        SVE::MaterialSettings materialSettings;
        materialSettings.name = "Skybox";
        materialSettings.vertexShaderName = "skyboxVertexShader";
        materialSettings.fragmentShaderName = "skyboxFragmentShader";
        materialSettings.textures = std::vector<SVE::TextureInfo> {
                /*{"samplerCubeMap", "textures/skybox/test/num_rt.png"},
                {"samplerCubeMap", "textures/skybox/test/num_lf.png"},
                {"samplerCubeMap", "textures/skybox/test/num_up.png"},
                {"samplerCubeMap", "textures/skybox/test/num_dn.png"},
                {"samplerCubeMap", "textures/skybox/test/num_ft.png"},
                {"samplerCubeMap", "textures/skybox/test/num_bk.png"}*/
                {"samplerCubeMap", "textures/skybox/blood_rt.png"},
                {"samplerCubeMap", "textures/skybox/blood_lf.png"},
                {"samplerCubeMap", "textures/skybox/blood_up.png"},
                {"samplerCubeMap", "textures/skybox/blood_dn.png"},
                {"samplerCubeMap", "textures/skybox/blood_bk.png"},
                {"samplerCubeMap", "textures/skybox/blood_ft.png"}
        };
        materialSettings.isCubemap = true;
        materialSettings.invertCullFace = true;
        materialSettings.useDepthTest = false;

        std::shared_ptr<SVE::Material> material = std::make_shared<SVE::Material>(materialSettings);
        engine->getMaterialManager()->registerMaterial(material);
    }

    // create skybox
    engine->getSceneManager()->setSkybox("Skybox");

    // create mesh
    std::shared_ptr<SVE::Mesh> mesh = std::make_shared<SVE::Mesh>("trashman", "models/trashman.dae");
    engine->getMeshManager()->registerMesh(mesh);

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
    newNode2->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(50, 0, 0)));

    //std::shared_ptr<Renderer> renderer = std::make_shared<VulkanRenderer>(window, true);

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
            //renderer->drawFrame();

            updateNode(newNode);
        }
        prevTime = curTime;
    }

    //renderer->finishRendering();
    //renderer.reset();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}