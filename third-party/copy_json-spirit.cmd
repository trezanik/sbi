@echo off
pushd %~dp0

REM /================================
REM / setup variables
REM /================================

set JSON_SPIRIT_VERSION=4.08
set JSON_SPIRIT_PATH=..\..\json_spirit\%JSON_SPIRIT_VERSION%
set DEST_PATH=.\json_spirit

REM /================================
REM / wipe out old files
REM /================================
rd /s /q %DEST_PATH%

REM /================================
REM / copy inclusions
REM /================================

REM subfolder called json_spirit as all we need, so copy from parent
xcopy /E /H /I /Y /Q "%JSON_SPIRIT_PATH%\json_spirit\*.h" "%DEST_PATH%\json_spirit"

popd
