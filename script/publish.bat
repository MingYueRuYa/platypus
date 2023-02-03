@echo off

cd /d %~dp0

set PUBLISH_DIR=%~dp0Publish\

call compile.bat
call generate_package.bat %1
call copy_pdb.bat %1