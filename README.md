# ![](Assets/ICON/ICON.png)  Jason's_law

Application should be able to run on windows without build.

Just clone the repo with command
```
git clone --recurse-submodules
```
and run command
```
hotrun.bat
```
If you want to build the project make sure your have msvc 2019 or 2022 installed in you system (I've only tested these two version), then simply run
```
build.bat
```

Note that the first time you buid this project you need to build the raylib first, change the following line in ```build.bat```
```
REM call scripts\msvc-build-raylib.bat
```
to
```
call scripts\msvc-build-debug.bat
```
Then you can delete the above line.
