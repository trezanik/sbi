@echo off
pushd %~dp0

REM /================================
REM / setup variables
REM /================================

set JSON_SPIRIT_VERSION=4.05
set JSON_SPIRIT_PATH=..\..\json-spirit\%JSON_SPIRIT_VERSION%
set DEST_PATH=.\json_spirit
set LIB_PATH=%DEST_PATH%\lib

REM /================================
REM / wipe out old files
REM /================================
rd /s /q %DEST_PATH%

REM /================================
REM / copy inclusions
REM /================================

REM subfolder called json-spirit as all we need, so copy from parent
xcopy /E /H /I /Y /Q "%JSON_SPIRIT_PATH%\include" "%DEST_PATH%\"

REM /================================
REM / copy libraries
REM /================================

mkdir %LIB_PATH%
REM not built this yet, todo

popd
