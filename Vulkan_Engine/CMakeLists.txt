set(PROJECT_NAME Vulkan_Engine)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "src/vk_engine.h"
    "src/vk_types.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Header_Files__Engine
    "src/Engine/Bitmask.h"
    "src/Engine/RenderLoopStatistics.h"
    "src/Engine/Window.h"
)
source_group("Header Files/Engine" FILES ${Header_Files__Engine})

set(Header_Files__EngineCore
    "src/EngineCore/BuffersUBO.h"
    "src/EngineCore/BuffersUBOPool.h"
    "src/EngineCore/Camera.h"
    "src/EngineCore/CollectionUtility.h"
    "src/EngineCore/Color.h"
    "src/EngineCore/Common.h"
    "src/EngineCore/DescriptorPoolManager.h"
    "src/EngineCore/ImGuiHandle.h"
    "src/EngineCore/IndexAttributes.h"
    "src/EngineCore/Material.h"
    "src/EngineCore/Mesh.h"
    "src/EngineCore/pch.h"
    "src/EngineCore/PipelineBinding.h"
    "src/EngineCore/Renderer.h"
    "src/EngineCore/Scene.h"
    "src/EngineCore/ShaderSource.h"
    "src/EngineCore/StagingBufferPool.h"
    "src/EngineCore/Texture.h"
    "src/EngineCore/Transform.h"
    "src/EngineCore/UboAllocatorDelegate.h"
    "src/EngineCore/VertexAttributes.h"
    "src/EngineCore/VertexBinding.h"
)
source_group("Header Files/EngineCore" FILES ${Header_Files__EngineCore})

set(Header_Files__FileManager
    "src/FileManager/Directories.h"
    "src/FileManager/FileIO.h"
    "src/FileManager/Path.h"
)
source_group("Header Files/FileManager" FILES ${Header_Files__FileManager})

set(Header_Files__Interfaces
    "src/Interfaces/IRequireInitialization.h"
)
source_group("Header Files/Interfaces" FILES ${Header_Files__Interfaces})

set(Header_Files__Loaders__Model
    "src/Loaders/Model/Common.h"
    "src/Loaders/Model/Loader_ASSIMP.h"
    "src/Loaders/Model/Loader_OBJ.h"
    "src/Loaders/Model/ModelLoaderOptions.h"
)
source_group("Header Files/Loaders/Model" FILES ${Header_Files__Loaders__Model})

set(Header_Files__Math
    "src/Math/BoundsAABB.h"
    "src/Math/Frustum.h"
    "src/Math/Plane.h"
)
source_group("Header Files/Math" FILES ${Header_Files__Math})

set(Header_Files__Presentation
    "src/Presentation/Device.h"
    "src/Presentation/Frame.h"
    "src/Presentation/FrameCollection.h"
    "src/Presentation/HardwareDevice.h"
    "src/Presentation/PresentationTarget.h"
)
source_group("Header Files/Presentation" FILES ${Header_Files__Presentation})

set(Header_Files__Presentation__Passes
    "src/Presentation/Passes/DebugPass.h"
    "src/Presentation/Passes/Pass.h"
    "src/Presentation/Passes/ShadowmapPass.h"
)
source_group("Header Files/Presentation/Passes" FILES ${Header_Files__Presentation__Passes})

set(Header_Files__Profiling
    "src/Profiling/ProfileMarker.h"
)
source_group("Header Files/Profiling" FILES ${Header_Files__Profiling})

set(Header_Files__VkTypes
    "src/VkTypes/InitializersUtility.h"
    "src/VkTypes/PipelineConstructor.h"
    "src/VkTypes/PushConstantTypes.h"
    "src/VkTypes/VkGraphicsPipeline.h"
    "src/VkTypes/vkInitQueueFamily.h"
    "src/VkTypes/VkMaterialVariant.h"
    "src/VkTypes/VkMemoryAllocator.h"
    "src/VkTypes/VkMesh.h"
    "src/VkTypes/VkMeshRenderer.h"
    "src/VkTypes/VkShader.h"
    "src/VkTypes/VkTexture.h"
    "src/VkTypes/VulkanValidationLayers.h"
)
source_group("Header Files/VkTypes" FILES ${Header_Files__VkTypes})

set(Source_Files
    "src/vk_engine.cpp"
    "src/vk_types.cpp"
    "src/vkinit_instance.cpp"
    "src/vkinit_queue.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(Source_Files__Engine
    "src/Engine/Window.cpp"
)
source_group("Source Files/Engine" FILES ${Source_Files__Engine})

set(Source_Files__EngineCore
    "src/EngineCore/BuffersUBO.cpp"
    "src/EngineCore/BuffersUBOPool.cpp"
    "src/EngineCore/Camera.cpp"
    "src/EngineCore/Color.cpp"
    "src/EngineCore/DescriptorPoolManager.cpp"
    "src/EngineCore/ImGuiHandle.cpp"
    "src/EngineCore/IndexAttributes.cpp"
    "src/EngineCore/Material.cpp"
    "src/EngineCore/Mesh.cpp"
    "src/EngineCore/MeshPrimitives.cpp"
    "src/EngineCore/pch.cpp"
    "src/EngineCore/PipelineBinding.cpp"
    "src/EngineCore/Renderer.cpp"
    "src/EngineCore/Scene.cpp"
    "src/EngineCore/ShaderSource.cpp"
    "src/EngineCore/StagingBufferPool.cpp"
    "src/EngineCore/SubMesh.cpp"
    "src/EngineCore/Texture.cpp"
    "src/EngineCore/Transform.cpp"
    "src/EngineCore/UboAllocatorDelegate.cpp"
    "src/EngineCore/VertexAttributes.cpp"
    "src/EngineCore/VertexBinding.cpp"
)
source_group("Source Files/EngineCore" FILES ${Source_Files__EngineCore})

set(Source_Files__FileManager
    "src/FileManager/Directories.cpp"
    "src/FileManager/FileIO.cpp"
    "src/FileManager/Path.cpp"
)
source_group("Source Files/FileManager" FILES ${Source_Files__FileManager})

set(Source_Files__Loaders__Model
    "src/Loaders/Model/Common.cpp"
    "src/Loaders/Model/Loader_ASSIMP.cpp"
    "src/Loaders/Model/Loader_OBJ.cpp"
)
source_group("Source Files/Loaders/Model" FILES ${Source_Files__Loaders__Model})

set(Source_Files__Math
    "src/Math/BoundsAABB.cpp"
    "src/Math/Frustum.cpp"
    "src/Math/Plane.cpp"
)
source_group("Source Files/Math" FILES ${Source_Files__Math})

set(Source_Files__Presentation
    "src/Presentation/Device.cpp"
    "src/Presentation/Frame.cpp"
    "src/Presentation/FrameCollection.cpp"
    "src/Presentation/HardwareDevice.cpp"
)
source_group("Source Files/Presentation" FILES ${Source_Files__Presentation})

set(Source_Files__Presentation__Passes
    "src/Presentation/Passes/DebugPass.cpp"
    "src/Presentation/Passes/Pass.cpp"
    "src/Presentation/Passes/ShadowmapPass.cpp"
)
source_group("Source Files/Presentation/Passes" FILES ${Source_Files__Presentation__Passes})

set(Source_Files__Presentation__Target
    "src/Presentation/PresentationTarget.cpp"
    "src/Presentation/PresentationTarget_GraphicsMaterial.cpp"
    "src/Presentation/PresentationTarget_RenderLoop.cpp"
)
source_group("Source Files/Presentation/Target" FILES ${Source_Files__Presentation__Target})

set(Source_Files__Profiling
    "src/Profiling/ProfileMarker.cpp"
)
source_group("Source Files/Profiling" FILES ${Source_Files__Profiling})

set(Source_Files__VkTypes
    "src/VkTypes/InitializersUtility.cpp"
    "src/VkTypes/VkGraphicsPipeline.cpp"
    "src/VkTypes/VkMaterialVariant.cpp"
    "src/VkTypes/VkMemoryAllocator.cpp"
    "src/VkTypes/VkMesh.cpp"
    "src/VkTypes/VkMeshRenderer.cpp"
    "src/VkTypes/VkShader.cpp"
    "src/VkTypes/VkTexture.cpp"
    "src/VkTypes/VulkanValidationLayers.cpp"
)
source_group("Source Files/VkTypes" FILES ${Source_Files__VkTypes})

set(ALL_FILES
    ${Header_Files}
    ${Header_Files__Engine}
    ${Header_Files__EngineCore}
    ${Header_Files__FileManager}
    ${Header_Files__Interfaces}
    ${Header_Files__Loaders__Model}
    ${Header_Files__Math}
    ${Header_Files__Presentation}
    ${Header_Files__Presentation__Passes}
    ${Header_Files__Profiling}
    ${Header_Files__VkTypes}
    ${Source_Files}
    ${Source_Files__Engine}
    ${Source_Files__EngineCore}
    ${Source_Files__FileManager}
    ${Source_Files__Loaders__Model}
    ${Source_Files__Math}
    ${Source_Files__Presentation}
    ${Source_Files__Presentation__Passes}
    ${Source_Files__Presentation__Target}
    ${Source_Files__Profiling}
    ${Source_Files__VkTypes}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/EngineCore/pch.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE VulkanEngine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EngineCore;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Third_Party_Project;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/SDL2-2.0.22/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/assimp/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/glm;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost_1.80/include;"
    "${VULKAN_PATH}/Include"
)

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "X64;"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "WIN32;"
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        "_MBCS"
        "_CONSOLE"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG;"
    ">"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy;
                /Y-
            >
            /permissive-;
            /std:c++17;
            /sdl-;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy;
                /Y-
            >
            /permissive-;
            /std:c++17;
            /sdl-;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /SUBSYSTEM:CONSOLE
            >
            $<$<CONFIG:Release>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /SUBSYSTEM:CONSOLE;
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Third_Party_Project
)
