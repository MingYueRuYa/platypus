@echo off
setlocal EnableDelayedExpansion

set title====== generate qm files =====
set lang="zh_CN" "en_US"
set configs="Debug" "Release"
set rootdir=%~dp0\..\platypus
set langdir=%rootdir%\res\Language
set tool=f:/qt5.15.2/bin/bin/lrelease.exe

echo %title%
echo.

for %%a in (%lang%) do (
    call %tool% -compress "%langdir%\%%~a\platypus.ts" 

    set src=%langdir%\%%~a\platypus.qm
    if not exist "!src!" (
        echo.
        echo generate qm file error:!src! 
        call :fail
        exit /b 1
    )

    for %%c in (%configs%) do (
        
		set dst_dir=%rootdir%\bin\%%~c\config\language\%%~a\
		set dst=%dst_dir%\platypus.qm
		
		if not exist "!dst_dir!" (
			mkdir %dst_dir% 
		)
		
        attrib "!dst!" -a -s -r -h > nul
        copy  "!src!" "!dst!" > nul
		if not exist "!dst!" (
            echo.
            echo copy QM file error:!dst! 
            call :fail
            exit /b 2
        ) else (
            echo copy QM file successful:!dst! 
        )
    )

    del "!src!" > nul 2>nul
    echo.
)

echo %title% [ successful ]
call :clear
exit /b 0

:fail
echo.
echo %title%  [ failed ]
call :clear 0
goto :EOF


:clear
echo.
goto :EOF
