#!/bin/bash

defines="-DENGINE"
warnings="-Wno-writable-strings -Wno-format-security -Wno-deprecated-declarations -Wno-switch"
includes="-Ithird_party -Ithird_party/include"
libs="-Lthird_party/lib -lraylib -luser32 -lgdi32 -lshell32 -lmsvcrt -lwinmm -nostdlib "

outputFile=JasonLaw.exe

echo "Engine not running, building main..."
clang++ $includes -g src/main.cpp -o$outputFile  $libs $warnings $defines
