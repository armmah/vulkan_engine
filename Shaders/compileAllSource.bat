set "AllowExt=.vert .frag"
for %%a in (%AllowExt%) do (
  forfiles /p C:\Git\Vulkan_Engine\Shaders\sourceGLSL /m *%%a /c "cmd /c C:\VulkanSDK\1.3.211.0\Bin\glslc.exe @file -o @file.spv"
)

move C:\Git\Vulkan_Engine\Shaders\sourceGLSL\*.spv C:\Git\Vulkan_Engine\Resources\Library\outputSPV