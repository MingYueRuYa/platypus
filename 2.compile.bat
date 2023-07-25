@echo off
setlocal EnableDelayedExpansion

pushd %~dp0

set title====== Compiling solution ======
set script=.\script

echo %title%
echo.

:: call compile
call %script%\compile.bat
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

