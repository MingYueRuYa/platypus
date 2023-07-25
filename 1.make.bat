@echo off
setlocal EnableDelayedExpansion

pushd %~dp0

set title====== Generate plug-in [project file] and [solution] =====
set tool=.\.premake-qt\premake5.exe
set script=.\premake5.lua
set outdir=.\Build
set vsver=vs2019

echo %title%
echo.

call %tool% --file=%script% --to=%outdir% %vsver%
echo.

if !errorlevel! == 0 (
    call :done
    exit /b 0
) else (
    call :fail
    exit /b 1
)

:done
echo %title% [successed!]
goto :EOF


:fail
echo %title% [failed!]
goto :EOF

