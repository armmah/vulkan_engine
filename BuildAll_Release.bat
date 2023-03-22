set buildPath=_Build\Release

:: Create build folder
if not exist %~dp0%buildPath% mkdir %~dp0%buildPath%

:: Copy SDL2
robocopy %~dp0third_party\SDL2-2.0.22\Lib\x64\ %~dp0%buildPath% *.dll /S

:: Copy Assimp
robocopy %~dp0third_party\assimp\Lib\Release\ %~dp0%buildPath% *.dll /S

:: Copy Resources
robocopy %~dp0Resources\Library %~dp0%buildPath%\Resources\Library /E
robocopy %~dp0Resources\Serialized %~dp0%buildPath%\Resources\Serialized /E

:: Copy executable and libs
robocopy %~dp0x64\Release\ %~dp0%buildPath%\ *.exe /S