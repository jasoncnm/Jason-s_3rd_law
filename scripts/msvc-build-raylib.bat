@echo off

set LIBRARIES=kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
set RAYLIB=rcore.c rmodels.c raudio.c rglfw.c rshapes.c rtext.c rtextures.c -I.\external\glfw\include
set RAYLIB_DEFINES=/DBUILD_LIBTYPE_SHARED /DPLATFORM_DESKTOP

IF EXIST "bin\raylib.dll" (IF EXIST "bin\raylib.lib" (goto end))

call scripts\msvc_upgrade_cmd_64.bat

set LIBPATH=src\vendor\raylib\src

IF NOT EXIST "bin" mkdir bin

pushd %LIBPATH%

cl /LD /Zi /Od /DEBUG:FULL %RAYLIB_DEFINES%  /Fe: "raylib" %RAYLIB% /link %LIBRARIES%

popd

copy %LIBPATH%\raylib.h   src
copy %LIBPATH%\raymath.h  src

move %LIBPATH%\raylib.dll bin
move %LIBPATH%\raylib.lib bin
move %LIBPATH%\*.pdb bin

del  %LIBPATH%\*.obj
del  %LIBPATH%\*.exp


:end
