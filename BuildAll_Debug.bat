set configuration=Debug
set arch=x64
set buildPath=_Build\%configuration%

:: Create build folder
if not exist %~dp0%buildPath% mkdir %~dp0%buildPath%

:: Copy SDL2
robocopy %~dp0third_party\SDL2-2.0.22\Lib\%arch%\ %~dp0%buildPath% *.dll /S

:: Copy Assimp
robocopy %~dp0third_party\assimp\Lib\%configuration%\ %~dp0%buildPath% *.dll /S

:: Copy Resources
robocopy %~dp0Resources\Library %~dp0%buildPath%\Resources\Library /E
robocopy %~dp0Resources\Serialized %~dp0%buildPath%\Resources\Serialized /E

:: Copy executable and libs
robocopy %~dp0%arch%\%configuration%\ %~dp0%buildPath%\ *.exe *.lib *.pdb /S