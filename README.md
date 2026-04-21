Collection of examples to demonstrate the usage of the SDL_GPU API.

## Cloning
This repository contains submodules for some of the external dependencies, so when doing a fresh clone you need to clone recursively:

```
git clone --recursive https://github.com/PeteHuf/SDL_gpu_examples_glTF.git
```

Updating submodules manually:

```
git submodule init
git submodule update
```


## Build

```
cmake -S . -B build
cmake --build build
```

The shaders in the repository are written in HLSL and offline compiled from `Content/Shaders/Source` to `Content/Shaders/Compiled` using [SDL_shadercross](https://github.com/libsdl-org/SDL_shadercross). If you want to build the shaders yourself, you must install `SDL_shadercross`, navigate to the shader source directory, and call `compile.sh`.

## PETEHUF_TODO:

* import glTF support libs
* port vkglTF from https://github.com/SaschaWillems/Vulkan-glTF-PBR via https://github.com/PeteHuf/SDL-glTF-PBR
* load geometry
* load textures
* load animation
