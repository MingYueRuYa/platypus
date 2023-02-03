@echo off

REM Use 7z to generate a compressed package
REM We need to exclude some files like pdb, exp etc.

cd /d %~dp0

%~dp0/7z/7z.exe a  %PUBLISH_DIR%/%1/%1.7z %~dp0/../bin_win32/Release/ -xr!*.lib -xr!*.exp -xr!*.pdb -xr!logs

echo "generate pack finished."