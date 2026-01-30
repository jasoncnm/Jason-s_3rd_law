@echo off

if "%1" == "clean" (del /Q bin\*)

call scripts\msvc_upgrade_cmd_64.bat
 
if "%1" == "release" (
	call scripts\msvc-build-release.bat
) else  (
	call scripts\msvc-build-raylib.bat
	call scripts\msvc-build-debug.bat
)
