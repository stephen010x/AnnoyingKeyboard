@echo off

if "%1"=="" goto :release
if "%1"=="release" goto :release
if "%1"=="console" goto :debug
echo "invalid parameter"
goto :eof

:release
::mkdir bin
set "buildcmd=gcc main.c -o bin/AnnoyingKeyboard.exe -Os -mwindows"
echo %buildcmd%
echo.
cmd /c "%buildcmd%"
goto :eof

:debug
set "buildcmd=gcc main.c -o run.exe -g -Og"
echo %buildcmd%
echo.
cmd /c "%buildcmd%"
goto :eof
