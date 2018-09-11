cmake_minimum_required(VERSION 3.12)
project(Chewman)

include_directories(deps/include)

link_directories(deps/lib)
link_directories(deps/lib/Vulkan)
link_directories(deps/lib/assimp)

set(CMAKE_CXX_STANDARD 14)

add_executable(Chewman main.cpp Renderer.cpp Renderer.h VulkanRenderer.cpp VulkanRenderer.h SVE/Engine.cpp SVE/Engine.h SVE/VulkanInstance.cpp SVE/VulkanInstance.h SVE/EngineSettings.cpp SVE/EngineSettings.h SVE/VulkanException.cpp SVE/VulkanException.h SVE/VulkanUtils.cpp SVE/VulkanUtils.h SVE/Entity.cpp SVE/Entity.h SVE/MeshEntity.cpp SVE/MeshEntity.h SVE/Material.cpp SVE/Material.h SVE/MeshSettings.cpp SVE/MeshSettings.h SVE/Libs.h SVE/VulkanMesh.cpp SVE/VulkanMesh.h SVE/VulkanMaterial.cpp SVE/VulkanMaterial.h SVE/MaterialManager.cpp SVE/MaterialManager.h SVE/SceneManager.cpp SVE/SceneManager.h SVE/SceneNode.cpp SVE/SceneNode.h SVE/MaterialSettings.cpp SVE/MaterialSettings.h SVE/ShaderInfo.cpp SVE/ShaderInfo.h SVE/VulkanShaderInfo.cpp SVE/VulkanShaderInfo.h SVE/ShaderSettings.cpp SVE/ShaderSettings.h SVE/ShaderManager.cpp SVE/ShaderManager.h SVE/CameraNode.cpp SVE/CameraNode.h SVE/CameraSettings.cpp SVE/CameraSettings.h SVE/MeshManager.cpp SVE/MeshManager.h SVE/Mesh.cpp SVE/Mesh.h)
target_link_libraries(Chewman mingw32 SDL2main SDL2 vulkan-1.lib libassimp )