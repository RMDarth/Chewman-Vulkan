LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
                    $(LOCAL_PATH)/deps/include

LOCAL_CPP_FEATURES := exceptions

# Add your application source files here...
LOCAL_SRC_FILES := vulkan_wrapper.cpp \
    AndroidFS.h \
    AndroidFS.cpp \
    SVE/CameraNode.cpp \
    SVE/CameraNode.h \
    SVE/CameraSettings.cpp \
    SVE/CameraSettings.h \
    SVE/ComputeEntity.cpp \
    SVE/ComputeEntity.h \
    SVE/ComputeSettings.cpp \
    SVE/ComputeSettings.h \
    SVE/Engine.cpp \
    SVE/Engine.h \
    SVE/EngineSettings.cpp \
    SVE/EngineSettings.h \
    SVE/Entity.cpp \
    SVE/Entity.h \
    SVE/FontManager.cpp \
    SVE/FontManager.h \
    SVE/Libs.h \
    SVE/LightManager.cpp \
    SVE/LightManager.h \
    SVE/LightNode.cpp \
    SVE/LightNode.h \
    SVE/LightSettings.h \
    SVE/Material.cpp \
    SVE/Material.h \
    SVE/MaterialManager.cpp \
    SVE/MaterialManager.h \
    SVE/MaterialSettings.cpp \
    SVE/MaterialSettings.h \
    SVE/Mesh.cpp \
    SVE/Mesh.h \
    SVE/MeshDefs.h \
    SVE/MeshEntity.cpp \
    SVE/MeshEntity.h \
    SVE/MeshManager.cpp \
    SVE/MeshManager.h \
    SVE/MeshSettings.cpp \
    SVE/MeshSettings.h \
    SVE/OverlayEntity.cpp \
    SVE/OverlayEntity.h \
    SVE/OverlayManager.cpp \
    SVE/OverlayManager.h \
    SVE/OverlaySettings.h \
    SVE/ParticleSystemEntity.cpp \
    SVE/ParticleSystemEntity.h \
    SVE/ParticleSystemManager.cpp \
    SVE/ParticleSystemManager.h \
    SVE/ParticleSystemSettings.cpp \
    SVE/ParticleSystemSettings.h \
    SVE/PipelineCacheManager.cpp \
    SVE/PipelineCacheManager.h \
    SVE/PostEffectManager.cpp \
    SVE/PostEffectManager.h \
    SVE/ResourceManager.cpp \
    SVE/ResourceManager.h \
    SVE/SceneManager.cpp \
    SVE/SceneManager.h \
    SVE/SceneNode.cpp \
    SVE/SceneNode.h \
    SVE/ShaderInfo.cpp \
    SVE/ShaderInfo.h \
    SVE/ShaderManager.cpp \
    SVE/ShaderManager.h \
    SVE/ShaderSettings.cpp \
    SVE/ShaderSettings.h \
    SVE/ShadowMap.cpp \
    SVE/ShadowMap.h \
    SVE/Skybox.cpp \
    SVE/Skybox.h \
    SVE/TextEntity.cpp \
    SVE/TextEntity.h \
    SVE/TextSettings.h \
    SVE/Utils.h \
    SVE/VulkanCommandsManager.h \
    SVE/VulkanComputeEntity.cpp \
    SVE/VulkanComputeEntity.h \
    SVE/VulkanDirectShadowMap.cpp \
    SVE/VulkanDirectShadowMap.h \
    SVE/VulkanException.cpp \
    SVE/VulkanException.h \
    SVE/VulkanInstance.cpp \
    SVE/VulkanInstance.h \
    SVE/VulkanMaterial.cpp \
    SVE/VulkanMaterial.h \
    SVE/VulkanMesh.cpp \
    SVE/VulkanMesh.h \
    SVE/VulkanParticleSystem.cpp \
    SVE/VulkanParticleSystem.h \
    SVE/VulkanPassInfo.cpp \
    SVE/VulkanPassInfo.h \
    SVE/VulkanPointShadowMap.cpp \
    SVE/VulkanPointShadowMap.h \
    SVE/VulkanPostEffect.cpp \
    SVE/VulkanPostEffect.h \
    SVE/VulkanSamplerHolder.cpp \
    SVE/VulkanSamplerHolder.h \
    SVE/VulkanScreenQuad.cpp \
    SVE/VulkanScreenQuad.h \
    SVE/VulkanShaderInfo.cpp \
    SVE/VulkanShaderInfo.h \
    SVE/VulkanUtils.cpp \
    SVE/VulkanUtils.h \
    SVE/VulkanWater.cpp \
    SVE/VulkanWater.h \
    SVE/Water.cpp \
    SVE/Water.h \
    Game/Controls/BoxSliderControl.cpp \
    Game/Controls/BoxSliderControl.h \
    Game/Controls/ButtonControl.cpp \
    Game/Controls/ButtonControl.h \
    Game/Controls/ContainerControl.cpp \
    Game/Controls/ContainerControl.h \
    Game/Controls/Control.cpp \
    Game/Controls/Control.h \
    Game/Controls/ControlDocument.cpp \
    Game/Controls/ControlDocument.h \
    Game/Controls/ControlFactory.cpp \
    Game/Controls/ControlFactory.h \
    Game/Controls/DropDownControl.cpp \
    Game/Controls/DropDownControl.h \
    Game/Controls/IEventHandler.h \
    Game/Controls/ImageControl.cpp \
    Game/Controls/ImageControl.h \
    Game/Controls/LabelControl.cpp \
    Game/Controls/LabelControl.h \
    Game/Controls/LevelButtonControl.cpp \
    Game/Controls/LevelButtonControl.h \
    Game/Controls/PanelControl.cpp \
    Game/Controls/PanelControl.h \
    Game/Controls/SliderControl.cpp \
    Game/Controls/SliderControl.h \
    Game/IGameMapService.h \
    Game/Game.cpp \
    Game/Game.h \
    Game/GameDefs.h \
	Game/GameSettings.cpp \
	Game/GameSettings.h \
    Game/GameSoundsManager.cpp \
    Game/GameSoundsManager.h \
    Game/Level/BlockMeshGenerator.cpp \
    Game/Level/BlockMeshGenerator.h \
    Game/Level/Bomb.cpp \
    Game/Level/Bomb.h \
    Game/Level/Coin.h \
    Game/Level/EatEffectManager.cpp \
    Game/Level/EatEffectManager.h \
    Game/Level/FireLineEntity.cpp \
    Game/Level/FireLineEntity.h \
    Game/Level/Enemies/Angel.cpp \
    Game/Level/Enemies/Angel.h \
    Game/Level/Enemies/ChewmanAI.cpp \
    Game/Level/Enemies/ChewmanAI.h \
    Game/Level/Enemies/ChewmanEnemy.cpp \
    Game/Level/Enemies/ChewmanEnemy.h \
    Game/Level/Enemies/DefaultEnemy.cpp \
    Game/Level/Enemies/DefaultEnemy.h \
    Game/Level/Enemies/Enemy.cpp \
    Game/Level/Enemies/Enemy.h \
    Game/Level/Enemies/EnemyAI.h \
    Game/Level/Enemies/HuntAI.cpp \
    Game/Level/Enemies/HuntAI.h \
    Game/Level/Enemies/Knight.cpp \
    Game/Level/Enemies/Knight.h \
    Game/Level/Enemies/Nun.cpp \
    Game/Level/Enemies/Nun.h \
    Game/Level/Enemies/Projectile.cpp \
    Game/Level/Enemies/Projectile.h \
    Game/Level/Enemies/RandomWalkerAI.cpp \
    Game/Level/Enemies/RandomWalkerAI.h \
    Game/Level/Enemies/Witch.cpp \
    Game/Level/Enemies/Witch.h \
    Game/Level/GameMap.cpp \
    Game/Level/GameMap.h \
    Game/Level/GameMapDefs.h \
    Game/Level/GameMapLoader.cpp \
    Game/Level/GameMapLoader.h \
    Game/Level/GameRulesProcessor.cpp \
    Game/Level/GameRulesProcessor.h \
    Game/Level/GameUtils.cpp \
    Game/Level/GameUtils.h \
    Game/Level/Gargoyle.h \
    Game/Level/LevelStateProcessor.cpp \
    Game/Level/LevelStateProcessor.h \
    Game/Level/MapTraveller.cpp \
    Game/Level/MapTraveller.h \
    Game/Level/Player.cpp \
    Game/Level/Player.h \
    Game/Level/PowerUp.cpp \
    Game/Level/PowerUp.h \
    Game/Level/StaticObject.cpp \
    Game/Level/StaticObject.h \
    Game/Level/Teleport.h \
    Game/Menu/CreditsStateProcessor.cpp \
    Game/Menu/CreditsStateProcessor.h \
    Game/Menu/GraphicsStateProcessor.cpp \
    Game/Menu/GraphicsStateProcessor.h \
    Game/Menu/HighScoresStateProcessor.cpp \
    Game/Menu/HighScoresStateProcessor.h \
    Game/Menu/LevelSelectionStateProcessor.cpp \
    Game/Menu/LevelSelectionStateProcessor.h \
	Game/Menu/MapStateProcessor.cpp \
	Game/Menu/MapStateProcessor.h \
    Game/Menu/MenuStateProcessor.cpp \
    Game/Menu/MenuStateProcessor.h \
    Game/Menu/PauseStateProcessor.cpp \
    Game/Menu/PauseStateProcessor.h \
    Game/Menu/ReviveStateProcessor.cpp \
    Game/Menu/ReviveStateProcessor.h \
    Game/Menu/ScoreStateProcessor.cpp \
    Game/Menu/ScoreStateProcessor.h \
    Game/Menu/SettingsStateProcessor.cpp \
    Game/Menu/SettingsStateProcessor.h \
    Game/Menu/TutorialStateProcessor.cpp \
    Game/Menu/TutorialStateProcessor.h \
    Game/Menu/WorldSelectionStateProcessor.cpp \
    Game/Menu/WorldSelectionStateProcessor.h \
    Game/ProgressManager.cpp \
    Game/ProgressManager.h \
    Game/StateProcessor.cpp \
    Game/StateProcessor.h \
    Game/GraphicsSettings.cpp \
    Game/GraphicsSettings.h \
    Game/ScoresManager.cpp \
    Game/ScoresManager.h \
    Game/SoundSystem.h \
    SoundSystemAndroid.cpp \
    Game/SystemApi.h \
    Game/SystemApi.cpp \
    Game/Utils.cpp \
    Game/Utils.h \
    main.cpp

LOCAL_SHARED_LIBRARIES := SDL2 assimp tinyxml2
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -landroid
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := assimp
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_PATH)/deps/lib/$(TARGET_ARCH_ABI)/libassimp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tinyxml2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_PATH)/deps/lib/$(TARGET_ARCH_ABI)/libtinyxml2.so
include $(PREBUILT_SHARED_LIBRARY)