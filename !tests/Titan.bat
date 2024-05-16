@echo off
setlocal enabledelayedexpansion

rem Definition of architectures and configurations
set "architectures[1]=64 bit"
set "architectures[2]=32 bit"
set "architectures[3]=ARM"
set "architectures[4]=Web"
set "architectures[5]=Nintendo Switch"
set "architectures[6]=Android"

set "configurations[1]=Debug DX11"
set "configurations[2]=Debug GL"
set "configurations[3]=Debug Universal DX11"
set "configurations[4]=Release DX11"
set "configurations[5]=Release GL"
set "configurations[6]=Release Universal DX11"

echo Veuillez entrer l'architecture :
for /l %%i in (1,1,6) do (
    echo %%i !architectures[%%i]!
)

set /p architectureIndex="Enter the number corresponding to your choice : "
set "architecture=!architectures[%architectureIndex%]!"

echo.
echo Veuillez entrer la configuration de build :
for /l %%i in (1,1,6) do (
    echo %%i !configurations[%%i]!
)

set /p configurationIndex="Enter the number corresponding to your choice : "
set "configuration=!configurations[%configurationIndex%]!"

echo.
msbuild /p:Configuration=!configuration! /p:Platform=!architecture! Titan.sln

endlocal
