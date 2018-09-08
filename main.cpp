#include <iostream>
#include "SDL2/SDL.h"
#include "VulkanRenderer.h"
#include "SVE/Engine.h"
#include "SVE/Material.h"
#include "SVE/MeshEntity.h"
#include "SVE/MaterialManager.h"
#include "SVE/SceneManager.h"
#include <vulkan/vulkan.h>
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

    SVE::MaterialSettings materialSettings;
    materialSettings.name = "material";
    std::shared_ptr<SVE::Material> material = std::make_shared<SVE::Material>(materialSettings);
    engine->getMaterialManager()->registerMaterial(material);

    std::shared_ptr<SVE::MeshEntity> meshEntity = std::make_shared<SVE::MeshEntity>("models/trashman.fbx");
    engine->getSceneManager()->getRootNode()->attachEntity(meshEntity);

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