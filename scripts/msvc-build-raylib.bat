@echo off

set LIBRARIES=kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
set RAYLIB=rcore.c rmodels.c raudio.c rglfw.c rshapes.c rtext.c rtextures.c utils.c -I.\external\glfw\include
set RAYLIB_DEFINES=/DBUILD_LIBTYPE_SHARED /DPLATFORM_DESKTOP

call msvc_upgrade_cmd_64.bat

set LIBPATH=..\src\vendor\raylib\src

mkdir ..\bin

pushd %LIBPATH%

cl /LD %RAYLIB_DEFINES%  /Fe: "raylib" %RAYLIB% /link %LIBRARIES%

popd

copy %LIBPATH%\raylib.h   %LIBPATH%\..
copy %LIBPATH%\raymath.h  %LIBPATH%\..

copy %LIBPATH%\raylib.dll ..\bin
copy %LIBPATH%\raylib.lib ..\bin
