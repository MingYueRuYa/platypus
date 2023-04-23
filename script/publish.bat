@echo off

REM publish.bat package_name

cd /d %~dp0
set cur_dir=%~dp0
set PUBLISH_DIR=%~dp0Publish\

call compile.bat
call generate_package.bat %1
call copy_pdb.bat %1

cd /d %cur_dir%
