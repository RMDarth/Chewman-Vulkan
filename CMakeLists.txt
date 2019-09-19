cmake_minimum_required(VERSION 3.10)
project(Chewman)

include_directories(./deps/include)
include_directories(.)

link_directories(./deps/lib)
link_directories(./deps/lib/Vulkan)
link_directories(./deps/lib/assimp)
link_directories(./deps/lib/cppfs)

set(CMAKE_CXX_STANDARD 14)

add_executable(Chewman
        main.cpp
        SVE/Engine.cpp
        SVE/Engine.h
        SVE/VulkanInstance.cpp
        SVE/VulkanInstance.h
        SVE/EngineSettings.cpp
        SVE/EngineSettings.h
        SVE/VulkanException.cpp
        SVE/VulkanException.h
        SVE/VulkanUtils.cpp
        SVE/VulkanUtils.h
        SVE/Entity.cpp
        SVE/Entity.h
        SVE/MeshEntity.cpp
        SVE/MeshEntity.h
        SVE/Material.cpp
        SVE/Material.h
        SVE/MeshSettings.cpp
        SVE/MeshSettings.h
        SVE/Libs.h
        SVE/VulkanMesh.cpp
        SVE/VulkanMesh.h
        SVE/VulkanMaterial.cpp
        SVE/VulkanMaterial.h
        SVE/MaterialManager.cpp
        SVE/MaterialManager.h
        SVE/SceneManager.cpp
        SVE/SceneManager.h
        SVE/SceneNode.cpp
        SVE/SceneNode.h
        SVE/MaterialSettings.cpp
        SVE/MaterialSettings.h
        SVE/ShaderInfo.cpp
        SVE/ShaderInfo.h
        SVE/VulkanShaderInfo.cpp
        SVE/VulkanShaderInfo.h
        SVE/ShaderSettings.cpp
        SVE/ShaderSettings.h
        SVE/ShaderManager.cpp
        SVE/ShaderManager.h
        SVE/CameraNode.cpp
        SVE/CameraNode.h
        SVE/CameraSettings.cpp
        SVE/CameraSettings.h
        SVE/MeshManager.cpp
        SVE/MeshManager.h
        SVE/Mesh.cpp
        SVE/Mesh.h
        SVE/LightNode.cpp
        SVE/LightNode.h
        SVE/LightSettings.h
        SVE/Skybox.cpp
        SVE/Skybox.h
        SVE/ResourceManager.cpp
        SVE/ResourceManager.h
        SVE/VulkanDirectShadowMap.cpp
        SVE/VulkanDirectShadowMap.h
        SVE/ShadowMap.cpp
        SVE/ShadowMap.h
        SVE/VulkanWater.cpp
        SVE/VulkanWater.h
        SVE/Water.cpp
        SVE/Water.h
        SVE/Utils.h
        SVE/VulkanScreenQuad.cpp
        SVE/VulkanScreenQuad.h
        SVE/LightManager.cpp
        SVE/LightManager.h
        SVE/VulkanSamplerHolder.cpp
        SVE/VulkanSamplerHolder.h
        SVE/VulkanPassInfo.cpp
        SVE/VulkanPassInfo.h
        SVE/VulkanPointShadowMap.cpp
        SVE/VulkanPointShadowMap.h
        SVE/VulkanCommandsManager.h
        SVE/VulkanComputeEntity.cpp
        SVE/VulkanComputeEntity.h
        SVE/ComputeSettings.cpp
        SVE/ComputeSettings.h
        SVE/ParticleSystemEntity.cpp
        SVE/ParticleSystemEntity.h
        SVE/VulkanParticleSystem.cpp
        SVE/VulkanParticleSystem.h
        SVE/ParticleSystemSettings.cpp
        SVE/ParticleSystemSettings.h
        Game/BlockMeshGenerator.cpp
        Game/BlockMeshGenerator.h
        Game/GameMap.cpp
        Game/GameMap.h
        SVE/ParticleSystemManager.cpp
        SVE/ParticleSystemManager.h
        SVE/ComputeEntity.cpp
        SVE/ComputeEntity.h
        Game/Gargoyle.h
        Game/Teleport.h
        Game/Coin.h
        SVE/PostEffectManager.cpp
        SVE/PostEffectManager.h
        SVE/VulkanPostEffect.cpp
        SVE/VulkanPostEffect.h
        Game/MapTraveller.cpp
        Game/MapTraveller.h
        Game/GameDefs.h
        Game/Enemies/Enemy.cpp
        Game/Enemies/Enemy.h
        Game/Enemies/EnemyAI.h
        Game/Enemies/Nun.cpp
        Game/Enemies/Nun.h
        Game/Enemies/RandomWalkerAI.cpp
        Game/Enemies/RandomWalkerAI.h
        Game/GameMapLoader.cpp
        Game/GameMapLoader.h
        Game/GameUtils.cpp
        Game/GameUtils.h Game/PowerUp.cpp Game/PowerUp.h Game/StaticObject.cpp Game/StaticObject.h Game/Player.cpp Game/Player.h SVE/TextEntity.cpp SVE/TextEntity.h Game/GameRulesProcessor.cpp Game/GameRulesProcessor.h SVE/TextSettings.h SVE/FontManager.cpp SVE/FontManager.h)

if (WIN32)
    target_link_libraries(Chewman mingw32 SDL2main SDL2 vulkan-1.lib VkLayer_core_validation.lib libassimp libcppfsd)
endif(WIN32)

if (UNIX)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    find_package(assimp REQUIRED)
    include_directories(${assimp_INCLUDE_DIRS})

    find_package(cppfs REQUIRED)
    include_directories(${cppfs_INCLUDE_DIRS})

    target_link_libraries(Chewman ${SDL2_LIBRARIES} assimp cppfs vulkan)
endif(UNIX)
