@echo off

set LINKER_FLAGS=/link -incremental:no -opt:ref raylib.lib kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
set COMMON_FLAGS=/std:c++17 /DGAME_INTERNAL=0 -Ox -O2 -Os -Ot -GL -Oi -FC
set EXE_NAME=game.exe
set INCLUDES=-I..\src\vendor\raylib\ -I..\src\vendor\raygui\src\

cd bin

cl %INCLUDES% ..\src\main.cpp /Fe:%EXE_NAME% %COMMON_FLAGS% %LINKER_FLAGS% %WARNINGS% %DEFINES%

cd ..

REM Comments
REM /Zi   - generate debugger files
REM /Fe   - change file name
REM -incremental:no -opt:ref - https://docs.microsoft.com/en-us/cpp/build/reference/incremental-link-incrementally?view=vs-2019
