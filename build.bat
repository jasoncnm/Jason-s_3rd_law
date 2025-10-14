@echo off


REM You can compile with either msvc or gcc
REM Just swap the scripts here and you are home

REM You CAN SKIP building raylib if it's your second time running this script!
REM Just delete line bellow
REM call msvc-build-raylib.bat 

cd scripts

call msvc-build.bat

cd ..
