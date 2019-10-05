# Chewman Vulkan
[![Language](https://img.shields.io/badge/Language%20-C++14-blue.svg?style=flat-square)](https://github.com/RMDarth/Chewman-Vulkan/)
[![Version](https://img.shields.io/badge/Version%20-0.6a-orange.svg?style=flat-square)](https://github.com/RMDarth/Chewman-Vulkan/)
[![Build Status](https://travis-ci.org/RMDarth/Chewman-Vulkan.svg?branch=master)](https://travis-ci.org/RMDarth/Chewman-Vulkan)

A cross-platform 3D Pacman-style game written in pure Vulkan and C++, with minimal dependencies. 
Main dependencies: SDL (windows initialization and controls) and assimp (3D assets loading).

Game development is currently in progress (early stage, current version is 0.6a).

![Screenshot](https://github.com/RMDarth/Chewman-Vulkan/blob/master/Screenshot_20190826.png?raw=true)

## Technology
Game is using Vulkan API/SDK for graphics, SDL for windows creation and controls, 
Assimp library for 3d models importing and some other small libraries like glm, 
cppfs, stb and rapidjson. Common Vulkan bootstrapping and scene management files are 
separated in the SVE folder - "Simple Vulkan Engine". 

SVE is a small game graphics engine, used for Vulkan initialization and processing, 
and some common game features - scene management, resources loader, materials, lights, 
particles and shaders management etc. Design of the engine is somewhat similar to other
 engines like Ogre or Irrlicht, but it's still pretty simple compared to those big 
 engines. SVE will probably move to a separate repository after the game is finished.
#### Main features of SVE
- Vulkan rendering
- Animation from loaded 3D assets
- Water rendering (reflection/refraction)
- Forward shading, point, direct and line lights
- Shadow maps, cascade shadow maps, point light shadows
- Instancing
- Particle system based on compute and geometry shaders
- Overlays and GUI rendering
- Post-effects (like bloom, grayscale)


## Game description
Game design is inspired by original Pacman game, but in a dark fantasy world. 

Game protagonist - Chewman - is a demon, who is hunting for treasure and lives
of innocent humans in the Maze. His enemies are Holy Order (knights, priests, 
nuns and angels) and other treasure hunters - witches and demons. Each enemy 
has some features and weaknesses. There are several power-ups on each map, 
like speed, move-through-walls, freeze enemies etc. Also there are teleports 
and traps like lava lakes, gargoyles breathing streams of fire etc. The objective 
is to collect all coins on the map.