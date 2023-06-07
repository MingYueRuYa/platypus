@echo off
setlocal EnableDelayedExpansion

:: 进入到整个项目的根目录（脚本当前目录）
pushd %~dp0

set title====== 生成插件的【工程文件】以及【解决方案】 =====
set tool=.\.premake-qt\premake5.exe
set script=.\premake5.lua
set outdir=.\Build
set vsver=vs2019

echo %title%
echo.

:: 生成工程文件和解决方案
call %tool% --file=%script% --to=%outdir% %vsver%
echo.

if !errorlevel! == 0 (
    call :done
    exit /b 0
) else (
    call :fail
    exit /b 1
)

:::::::::::::::::::: 以下为函数定义 ::::::::::::::::::::

:done
echo %title% 【成功！】
call :clear 5
goto :EOF


:fail
echo %title% 【失败！】
call :clear 0
goto :EOF

