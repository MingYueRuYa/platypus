@echo off

REM Usage: copy_pdb.bat [dir_name]
REM Copy pdb files for trobuleshooting

cd /d %~dp0

set dir_name="%1"
set abs_dir_name="%PUBLISH_DIR%"%dir_name%"\pdb"

IF NOT EXIST %abs_dir_name% (
    mkdir %abs_dir_name%
)

cd /d ../bin_win32/Release

xcopy /y /d *.pdb %abs_dir_name% 

echo "copy pdb files finished."