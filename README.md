# ![](Assets/ICON/ICON.png)  Jason's_law 

Jason's 3rd Law is a 2D grid-based puzzle game written in C++ using raylib.

The game is built around interactions between movable slimes and blocks, electrical systems, doors, portals, bridges, collectibles, and other environmental objects.

[![Watch the video](https://img.youtube.com/vi/DW0J4LAYtxA/hqdefault.jpg)](https://www.youtube.com/embed/DW0J4LAYtxA)

# Features

- Grid-based slime-like player movement
- Mass-dependent pushable and movable blocks
- Slime merging and splitting mechanics
- Electrical door circuit system
- Dynamic fog of war level reveal system
- Undo and restart functionality
- Tween-based animations for entity movement and camera transition
- Post-processing visual effects
- Live code editing in Windows by compiling gameplay code into a DLL
- JSON-based tilemap level creation

# How to Build and Run
Clone the repo with the command
```
git clone --recurse-submodules <URL> 
```
Make sure you have MSVC 2019 or 2022 installed on your system (I've only tested these two versions). Then, in the project directory, run
```
build
```
If you want a clean build, run
```
build clean
```
Then, to run the game, type
```
hotrun.bat
```
Alternatively, Go to the latest successful [Actions](https://github.com/jasoncnm/Jason-s_3rd_law/actions) and download and the 'release-file' artifacts. And Run 
```
game.exe
```
