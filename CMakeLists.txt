cmake_minimum_required(VERSION 3.10)
project(Chewman)

include_directories(./deps/include)
include_directories(.)

link_directories(./deps/lib)
link_directories(./deps/lib/Vulkan)
link_directories(./deps/lib/assimp)
link_directories(./deps/lib/cppfs)
link_directories(./deps/lib/tinyxml2)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/deps")
set(CMAKE_CXX_STANDARD 14)

add_executable(Chewman
        main.cpp
        DesktopFS.cpp
        DesktopFS.h
        VulkanHeaders.h
        SVE/CameraNode.cpp
        SVE/CameraNode.h
        SVE/CameraSettings.cpp
        SVE/CameraSettings.h
        SVE/ComputeEntity.cpp
        SVE/ComputeEntity.h
        SVE/ComputeSettings.cpp
        SVE/ComputeSettings.h
        SVE/Engine.cpp
        SVE/Engine.h
        SVE/EngineSettings.cpp
        SVE/EngineSettings.h
        SVE/Entity.cpp
        SVE/Entity.h
        SVE/FileSystem.h
        SVE/FontManager.cpp
        SVE/FontManager.h
        SVE/Libs.h
        SVE/LightManager.cpp
        SVE/LightManager.h
        SVE/LightNode.cpp
        SVE/LightNode.h
        SVE/LightSettings.h
        SVE/Material.cpp
        SVE/Material.h
        SVE/MaterialManager.cpp
        SVE/MaterialManager.h
        SVE/MaterialSettings.cpp
        SVE/MaterialSettings.h
        SVE/Mesh.cpp
        SVE/Mesh.h
        SVE/MeshDefs.h
        SVE/MeshEntity.cpp
        SVE/MeshEntity.h
        SVE/MeshManager.cpp
        SVE/MeshManager.h
        SVE/MeshSettings.cpp
        SVE/MeshSettings.h
        SVE/OverlayEntity.cpp
        SVE/OverlayEntity.h
        SVE/OverlayManager.cpp
        SVE/OverlayManager.h
        SVE/OverlaySettings.h
        SVE/ParticleSystemEntity.cpp
        SVE/ParticleSystemEntity.h
        SVE/ParticleSystemManager.cpp
        SVE/ParticleSystemManager.h
        SVE/ParticleSystemSettings.cpp
        SVE/ParticleSystemSettings.h
        SVE/PostEffectManager.cpp
        SVE/PostEffectManager.h
        SVE/ResourceManager.cpp
        SVE/ResourceManager.h
        SVE/SceneManager.cpp
        SVE/SceneManager.h
        SVE/SceneNode.cpp
        SVE/SceneNode.h
        SVE/ShaderInfo.cpp
        SVE/ShaderInfo.h
        SVE/ShaderManager.cpp
        SVE/ShaderManager.h
        SVE/ShaderSettings.cpp
        SVE/ShaderSettings.h
        SVE/ShadowMap.cpp
        SVE/ShadowMap.h
        SVE/Skybox.cpp
        SVE/Skybox.h
        SVE/TextEntity.cpp
        SVE/TextEntity.h
        SVE/TextSettings.h
        SVE/Utils.h
        SVE/VulkanCommandsManager.h
        SVE/VulkanComputeEntity.cpp
        SVE/VulkanComputeEntity.h
        SVE/VulkanDirectShadowMap.cpp
        SVE/VulkanDirectShadowMap.h
        SVE/VulkanException.cpp
        SVE/VulkanException.h
        SVE/VulkanInstance.cpp
        SVE/VulkanInstance.h
        SVE/VulkanMaterial.cpp
        SVE/VulkanMaterial.h
        SVE/VulkanMesh.cpp
        SVE/VulkanMesh.h
        SVE/VulkanParticleSystem.cpp
        SVE/VulkanParticleSystem.h
        SVE/VulkanPassInfo.cpp
        SVE/VulkanPassInfo.h
        SVE/VulkanPointShadowMap.cpp
        SVE/VulkanPointShadowMap.h
        SVE/VulkanPostEffect.cpp
        SVE/VulkanPostEffect.h
        SVE/VulkanSamplerHolder.cpp
        SVE/VulkanSamplerHolder.h
        SVE/VulkanScreenQuad.cpp
        SVE/VulkanScreenQuad.h
        SVE/VulkanShaderInfo.cpp
        SVE/VulkanShaderInfo.h
        SVE/VulkanUtils.cpp
        SVE/VulkanUtils.h
        SVE/VulkanWater.cpp
        SVE/VulkanWater.h
        SVE/Water.cpp
        SVE/Water.h
        Game/Controls/ButtonControl.cpp
        Game/Controls/ButtonControl.h
        Game/Controls/ContainerControl.cpp
        Game/Controls/ContainerControl.h
        Game/Controls/Control.cpp
        Game/Controls/Control.h
        Game/Controls/ControlDocument.cpp
        Game/Controls/ControlDocument.h
        Game/Controls/ControlFactory.cpp
        Game/Controls/ControlFactory.h
        Game/Controls/DropDownControl.cpp
        Game/Controls/DropDownControl.h
        Game/Controls/IEventHandler.h
        Game/Controls/ImageControl.cpp
        Game/Controls/ImageControl.h
        Game/Controls/LabelControl.cpp
        Game/Controls/LabelControl.h
        Game/Controls/LevelButtonControl.cpp
        Game/Controls/LevelButtonControl.h
        Game/Controls/PanelControl.cpp
        Game/Controls/PanelControl.h
        Game/Controls/SliderControl.cpp
        Game/Controls/SliderControl.h
        Game/Game.cpp
        Game/Game.h
        Game/GameDefs.h
        Game/GraphicsSettings.cpp
        Game/GraphicsSettings.h
        Game/Level/BlockMeshGenerator.cpp
        Game/Level/BlockMeshGenerator.h
        Game/Level/Bomb.cpp
        Game/Level/Bomb.h
        Game/Level/Coin.h
        Game/Level/EatEffectManager.cpp
        Game/Level/EatEffectManager.h
        Game/Level/FireLineEntity.cpp
        Game/Level/FireLineEntity.h
        Game/Level/Enemies/Angel.cpp
        Game/Level/Enemies/Angel.h
        Game/Level/Enemies/ChewmanEnemy.cpp
        Game/Level/Enemies/ChewmanEnemy.h
        Game/Level/Enemies/DefaultEnemy.cpp
        Game/Level/Enemies/DefaultEnemy.h
        Game/Level/Enemies/Enemy.cpp
        Game/Level/Enemies/Enemy.h
        Game/Level/Enemies/EnemyAI.h
        Game/Level/Enemies/HuntAI.cpp
        Game/Level/Enemies/HuntAI.h
        Game/Level/Enemies/Knight.cpp
        Game/Level/Enemies/Knight.h
        Game/Level/Enemies/Nun.cpp
        Game/Level/Enemies/Nun.h
        Game/Level/Enemies/Projectile.cpp
        Game/Level/Enemies/Projectile.h
        Game/Level/Enemies/RandomWalkerAI.cpp
        Game/Level/Enemies/RandomWalkerAI.h
        Game/Level/Enemies/Witch.cpp
        Game/Level/Enemies/Witch.h
        Game/Level/GameMap.cpp
        Game/Level/GameMap.h
        Game/Level/GameMapDefs.h
        Game/Level/GameMapLoader.cpp
        Game/Level/GameMapLoader.h
        Game/Level/GameRulesProcessor.cpp
        Game/Level/GameRulesProcessor.h
        Game/Level/GameUtils.cpp
        Game/Level/GameUtils.h
        Game/Level/Gargoyle.h
        Game/Level/LevelStateProcessor.cpp
        Game/Level/LevelStateProcessor.h
        Game/Level/MapTraveller.cpp
        Game/Level/MapTraveller.h
        Game/Level/Player.cpp
        Game/Level/Player.h
        Game/Level/PowerUp.cpp
        Game/Level/PowerUp.h
        Game/Level/StaticObject.cpp
        Game/Level/StaticObject.h
        Game/Level/Teleport.h
        Game/Menu/GraphicsStateProcessor.cpp
        Game/Menu/GraphicsStateProcessor.h
        Game/Menu/LevelSelectionStateProcessor.cpp
        Game/Menu/LevelSelectionStateProcessor.h
        Game/Menu/MenuStateProcessor.cpp
        Game/Menu/MenuStateProcessor.h
        Game/Menu/PauseStateProcessor.cpp
        Game/Menu/PauseStateProcessor.h
        Game/Menu/ScoreStateProcessor.cpp
        Game/Menu/ScoreStateProcessor.h
        Game/Menu/WorldSelectionStateProcessor.cpp
        Game/Menu/WorldSelectionStateProcessor.h
        Game/ProgressManager.cpp
        Game/ProgressManager.h
        Game/ScoresManager.cpp
        Game/ScoresManager.h
        Game/StateProcessor.cpp
        Game/StateProcessor.h)

if (WIN32)
    target_link_libraries(Chewman mingw32 SDL2main SDL2 vulkan-1.lib VkLayer_core_validation.lib libassimp libcppfsd libtinyxml2)
endif(WIN32)

if (UNIX)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    find_package(assimp REQUIRED)
    include_directories(${assimp_INCLUDE_DIRS})

    find_package(cppfs REQUIRED)
    include_directories(${cppfs_INCLUDE_DIRS})

    if (APPLE)
        find_package(vulkan REQUIRED)
        target_link_libraries(Chewman Vulkan::Vulkan)
    else (APPLE)
        find_package(TinyXML2 REQUIRED)
        include_directories(${TinyXML2_INCLUDE_DIRS})
    endif(APPLE)

    target_link_libraries(Chewman ${SDL2_LIBRARIES} assimp cppfs vulkan tinyxml2 pthread)
endif(UNIX)
