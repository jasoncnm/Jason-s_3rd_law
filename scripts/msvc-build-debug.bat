@echo off
REM /EXPORT:HotReload /EXPORT:HotUnload


set LINKER_FLAGS=/link raylib.lib kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib -incremental:no -opt:ref
set EXPORTED_FUNCTIONS=/EXPORT:UpdateAndRender
set COMMON_FLAGS=/std:c++17 /DGAME_INTERNAL=1 /Zi /nologo -GR- -Od -Oi -WX -W4 -wd4530 -wd4456 -wd4505 -wd4201 -wd4100 -wd4189 -wd4996 -FC
set EXE_NAME=game.exe
set DLL_NAME=game_code.dll
set INCLUDES=-I..\src\vendor\raylib\ -I..\src\vendor\raygui\src\

REM call msvc_upgrade_cmd_64.bat

cd bin

echo LOCKFILE IN AID OF HOTLOADING > lock.file
cl %INCLUDES% ..\src\game.cpp /LD /Fe:%DLL_NAME% %COMMON_FLAGS% %LINKER_FLAGS% %EXPORTED_FUNCTIONS% %WARNINGS% 
del lock.file
cl %INCLUDES% ..\src\main.cpp /D_AMD64_ /Fe:%EXE_NAME% %COMMON_FLAGS% %LINKER_FLAGS% %WARNINGS% %DEFINES%

cd ..

REM Comments
REM /LD   - create a dll file, dynamic library
REM /Zi   - generate debugger files
REM /Fe   - change file name
REM -incremental:no -opt:ref - https://docs.microsoft.com/en-us/cpp/build/reference/incremental-link-incrementally?view=vs-2019
REM -D_AMD64_ - define a _AMD64_ macro, wouldnt work without this
