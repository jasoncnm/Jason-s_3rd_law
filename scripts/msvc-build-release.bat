@echo off

set LINKER_FLAGS=/link -incremental:no -opt:ref kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
set COMMON_FLAGS=/std:c++17 /DGAME_INTERNAL=0 -Ox -O2 -Os -Ot -GL -Oi -FC /MT
set EXE_NAME=game.exe
set DEFINES=/DPLATFORM_DESKTOP

cd bin

set INCLUDES=-I..\src\vendor\raylib\ -I..\src\vendor\raygui\src\ -I..\src\vendor\raylib\src\external\glfw\include

set LIBPATH=..\src\vendor\raylib\src
set SOURCE_FILES=..\src\main.cpp %LIBPATH%\rcore.c %LIBPATH%\rmodels.c %LIBPATH%\raudio.c %LIBPATH%\rglfw.c %LIBPATH%\rshapes.c %LIBPATH%\rtext.c %LIBPATH%\rtextures.c

cl %DEFINES% %SOURCE_FILES% %INCLUDES%  /Fe:%EXE_NAME% %COMMON_FLAGS% %LINKER_FLAGS% %WARNINGS% 

cd ..

REM Comments
REM /Zi   - generate debugger files
REM /Fe   - change file name
REM -incremental:no -opt:ref - https://docs.microsoft.com/en-us/cpp/build/reference/incremental-link-incrementally?view=vs-2019
