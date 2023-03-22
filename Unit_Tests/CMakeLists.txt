set(PROJECT_NAME Unit_Tests)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "DataStructures/main.cpp"
)
source_group("" FILES ${no_group_source_files})

set(Serialization
    "test_binarySerialization.cpp"
)
source_group("Serialization" FILES ${Serialization})

set(ALL_FILES
    ${no_group_source_files}
    ${Serialization}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Main)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Vulkan_Engine/src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Vulkan_Engine/src/EngineCore;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/SDL2-2.0.22/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/assimp/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost_1.80/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/gtest/include;"
    "${VULKAN_PATH}/Include;"
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
    "UNICODE;"
    "_UNICODE"
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
        "_CONSOLE"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od;
                /RTC1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:Release>:
                /Zi
            >
            /std:c++17;
            /w;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od;
                /RTC1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:Release>:
                /Zi
            >
            /W3;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:Debug>:"
                "/Y-"
            ">"
        )
    endif()
    source_file_compile_options(DataStructures/main.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PUBLIC
    Vulkan_Engine
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "assimp-vc143-mtd;"
        "SDL2test;"
        "gtestd.lib;"
        "gtest_maind.lib;"
    ">"
    "$<$<CONFIG:Release>:"
        "assimp-vc143-mt;"
        "gtest.lib;"
        "gtest_main.lib;"
    ">"
    "vulkan-1.lib;"
    "SDL2;"
    "SDL2main"
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/third_party/SDL2-2.0.22/lib/${CMAKE_VS_PLATFORM_NAME};"
    "${CMAKE_SOURCE_DIR}/third_party/boost_1.80/Lib/msvc-14.3/$<CONFIG>;"
    "${CMAKE_SOURCE_DIR}/third_party/assimp/Lib/$<CONFIG>;"
    "${CMAKE_SOURCE_DIR}/third_party/gtest/Lib/140/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>;"
    "${VULKAN_PATH}/Lib;"
)
