@echo off
REM ######################################################################################
REM # Enables the DLL dependencies to be present in the current directory when debugging #
REM ######################################################################################

REM for usage on network drives/UNC paths to avoid hardcoding
pushd %~dp0

set BIN_PATH=Debug
set LIBCONFIG_PATH=..\..\third-party\libconfig-1.4.9
set QT_PATH=..\..\..\Qt\5.3\msvc2013_64

echo Copying DLL dependencies to %~dp0%BIN_PATH%...

REM >>> Libconfig
if not exist "%BIN_PATH%\libconfig++.dll" copy "%LIBCONFIG_PATH%\Debug\libconfig++.dll" "%BIN_PATH%\"
REM >>> Qt5
if not exist "%BIN_PATH%\Qt5Cored.dll" copy "%QT_PATH%\bin\Qt5Cored.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\Qt5Guid.dll" copy "%QT_PATH%\bin\Qt5Guid.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\Qt5Widgets.dll" copy "%QT_PATH%\bin\Qt5Widgets.dll" "%BIN_PATH%\"

popd
