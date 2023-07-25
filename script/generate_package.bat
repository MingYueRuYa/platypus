@echo off

REM Use 7z to generate a compressed package
REM We need to exclude some files like ilk, exp etc.

cd /d %~dp0

%~dp07z\7z.exe a  %PUBLISH_DIR%/%1/%1.7z %~dp0/../build/bin/Release/ -xr!*.lib -xr!*.exp -xr!logs -xr!*.ilk

echo "generate pack finished."