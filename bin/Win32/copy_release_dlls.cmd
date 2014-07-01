@echo off
REM ######################################################################################
REM # Enables the DLL dependencies to be present in the current directory when debugging #
REM ######################################################################################

REM for usage on network drives/UNC paths to avoid hardcoding
pushd %~dp0

set BIN_PATH=Release
set LIBCONFIG_PATH=..\..\..\libconfig\1.4.9
set QT_PATH=..\..\..\Qt\5.3\msvc2013

echo Copying DLL dependencies to %~dp0%BIN_PATH%...

REM @TODO:: get Qt built without need of the GL and other unused components

popd
