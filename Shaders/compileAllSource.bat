set "AllowExt=.vert .frag"

set arg1="%1\Bin\glslc.exe"
if not exist %arg1% (
    set arg1=C:\VulkanSDK\1.3.236.0\Bin\glslc.exe
)

for %%a in (%AllowExt%) do (
  forfiles /p %~dp0sourceGLSL /m *%%a /c "cmd /c %arg1% @file -o @file.spv"
)

move %~dp0sourceGLSL\*.spv  %~dp0..\Resources\Library\outputSPV