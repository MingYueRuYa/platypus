@echo off
setlocal EnableDelayedExpansion

set title====== generate ts files =====
set lang="zh_CN" "en_US"
set tool=f:/qt5.15.2/bin/bin/lupdate.exe

set cmdLine=-locations none -no-ui-lines -recursive
set cmdLine=!cmdLine! ..\platypus
::set cmdLine=!cmdLine! ..\dolphin
::set cmdLine=!cmdLine! -I .\include\AssistLib
pushd %~dp0

echo %title%
echo.

for %%a in (%lang%) do (
    set file=%cd%\..\platypus\res\Language\%%~a\platypus.ts

    attrib "!file!" -a -s -r -h > nul
    call %tool% !cmdLine! -ts "!file!"
    echo.

    if not !errorlevel! == 0 (
		echo "The generated ts file does not exist, and subsequent scripts stop executing, !file!" 
        call :fail
        exit /b 1
    )

    if not exist "!file!" (
		echo "The generated ts file does not exist, and subsequent scripts stop executing, !file!" 
        call :fail
        exit /b 2
    )
)

echo %title% [ succesful ]
call :clear
exit /b 0

:::::::::::::::::::: function define ::::::::::::::::::::

:fail
echo.
echo %title% [ failed ]
call :clear 0
goto :EOF


:clear
popd
goto :EOF

