@echo off
setlocal EnableDelayedExpansion

:: ���뵽������Ŀ�ĸ�Ŀ¼���ű���ǰĿ¼��
pushd %~dp0

set title====== ���ɲ���ġ������ļ����Լ������������ =====
set tool=.\.premake-qt\premake5.exe
set script=.\premake5.lua
set outdir=.\Build
set vsver=vs2019

echo %title%
echo.

:: ���ɹ����ļ��ͽ������
call %tool% --file=%script% --to=%outdir% %vsver%
echo.

if !errorlevel! == 0 (
    call :done
    exit /b 0
) else (
    call :fail
    exit /b 1
)

:::::::::::::::::::: ����Ϊ�������� ::::::::::::::::::::

:done
echo %title% ���ɹ�����
call :clear 5
goto :EOF


:fail
echo %title% ��ʧ�ܣ���
call :clear 0
goto :EOF

