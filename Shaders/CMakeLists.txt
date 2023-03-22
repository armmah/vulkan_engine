set(PROJECT_NAME Shaders)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "sourceGLSL/depthonly.vert"
    "sourceGLSL/quad.frag"
    "sourceGLSL/quad.vert"
    "sourceGLSL/simple.frag"
    "sourceGLSL/simple.vert"
    "sourceGLSL/triangle.frag"
    "sourceGLSL/triangle.vert"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

add_custom_target(${PROJECT_NAME} SOURCES ${ALL_FILES})

