## About

Screenshot.jpg

## Features

- Loading OBJ, GLTF,
- Custom serialization format,
- Supports DXT texture compression,
- Frustum Culling,
- Crytek sponza and Intel sponza scenes,
- Shadowmaps

## Dependencies

CMake 3.16.0 https://cmake.org/download/
VulkanSDK：https://www.lunarg.com/vulkan-sdk/
Visual Studio 2022: msvc-143
C++ version: 17

## Embed Libs

All embed libs can be found at (https://github.com/armmah/vulkan_engine/tree/main/third_party and https://github.com/armmah/vulkan_engine/tree/main/Third_Party_Project)

- assimp,
- boost 1.80,
- dds_loader,
- glm,
- imgui,
- SDL2,
- stb_image,
- tiny_gltf,
- tinyobjloader,
- vma,
- volk,
- (optional) gtest

## Build

1. Make sure that VulkanSDK is installed,
2. Run CMake using Visual Studio 2022 (for windows),
3. Open the solution with Visual Studio (for windows),
4. Set Main as the startup project,
5. Build the project,
- shaders,
- resources,
6. Run /BuildAll_Debug.bat,
7. See the build at /_Build/Debug/