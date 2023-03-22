## About

This project has been created for learning and familiriazing purposes of the Vulkan Graphics API.

Here are some screenshots from the application:

<img src="https://github.com/armmah/vulkan_engine/blob/main/Examples/Shadowmaps.gif"/>
![Intel sponza](https://github.com/armmah/vulkan_engine/blob/main/Examples/Shadowmaps.gif)

![Debugging depth](https://github.com/armmah/vulkan_engine/tree/main/Examples/Debugging_Depth.gif)

## Features

- Loading OBJ, GLTF,
- Custom serialization format,
- Supports DXT texture compression,
- Frustum Culling,
- Crytek sponza and Intel sponza scenes,
- Shadowmaps

## Dependencies

- CMake 3.16.0 https://cmake.org/download/
- VulkanSDK：https://www.lunarg.com/vulkan-sdk/
- Visual Studio 2022: msvc-143
- C++ version: 17

## Build

1. Make sure that VulkanSDK is installed,
2. Run CMake using Visual Studio 2022 (for windows),
3. Open the solution with Visual Studio (for windows),
4. Set Main as the startup project,
5. Build the project,
6. Run /BuildAll_Debug.bat,
7. See the build at /_Build/Debug/

## Build Details

For a successful build for the app to run without problems, the following things are necessary:
- The build .exe itself from (step 5),
- The DLLs ( SDL and assimp ), copied from third_party by BuildAll batch file (step 6),
- Resources/Serialized scene data with meshes and textures (step 6),
- Resources/Library spv shaders (step 6).

The above steps are automated by step 6.

Note that on the 5th step, the shaders are compiled as well via the batch file "Shaders/compileAllSource.bat". The repository also contains the default shaders so this step is not strictly necessary and you could create the cmake project with the ENABLE_AUTO_COMPILE_SHADERS option disabled.

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