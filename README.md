# ![](Assets/ICON/ICON.png)  Jason's_law

The application should be able to run on Windows without build.

Just clone the repo with the command
```
git clone --recurse-submodules <URL>
```
and run the command
```
hotrun.bat
```
If you want to build the project, make sure you have MSVC 2019 or 2022 installed in your system (I've only tested these two versions), then in the project directory, simply run
```
build clean
```
For the first time, or you want a clean build

Then, for subsequent builds, just run
```
build
```
