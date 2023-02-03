@echo off

SETLOCAL
SETLOCAL ENABLEDELAYEDEXPANSION

cd /d %~dp0

set VsWherePath="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"

SET VsWhereCmdLine="!VsWherePath! -nologo -latest -property installationPath"

FOR /F "usebackq delims=" %%i in (`!VsWhereCmdLine!`) DO (
    IF EXIST "%%i\VC\Auxiliary\Build\vcvars32.bat" (
        echo VS FOUND, %%i
		CALL "%%i\VC\Auxiliary\Build\vcvars32.bat"
        goto FOUND
    ) else (
		goto ERROR
	)
)


:FOUND
msbuild ../platypus.vcxproj -property:Configuration=Release;Platform=x86
msbuild ../git_plugin/git_plugin.vcxproj -property:Configuration=Release;Platform=x64
msbuild ../git_register_exec/git_register_exec.vcxproj -property:Configuration=Release;Platform=x64
exit /b 0

REM msbuild ./git_register_exec/git_register_exec.vcxproj -property:Configuration=Release;Platform=x64;CustomMacros=QTDESIGNER

:ERROR
echo "Not find vs."