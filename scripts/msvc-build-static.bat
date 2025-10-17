@echo off

set LINKER_FLAGS=/link raylib.lib kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib -incremental:no -opt:ref
set COMMON_FLAGS= /DBUILD_GAME_STATIC /DGAME_INTERNAL=0 /Zi /nologo -GR- -Od -Oi -WX -W4 -wd4530 -wd4456 -wd4505 -wd4201 -wd4100 -wd4189 -wd4996 -FC
set EXE_NAME=game.exe

cd bin

cl  ..\src\main.cpp /Fe:%EXE_NAME% %COMMON_FLAGS% %LINKER_FLAGS% %WARNINGS% %DEFINES%

cd ..

REM Comments
REM /Zi   - generate debugger files
REM /Fe   - change file name
REM -incremental:no -opt:ref - https://docs.microsoft.com/en-us/cpp/build/reference/incremental-link-incrementally?view=vs-2019
