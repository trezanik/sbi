@echo off
REM ######################################################################################
REM # Enables the DLL dependencies to be present in the current directory when debugging #
REM ######################################################################################

REM for usage on network drives/UNC paths to avoid hardcoding
pushd %~dp0

set BIN_PATH=Debug
set LIBCONFIG_PATH=..\..\..\libconfig\1.4.9
set QT_PATH=..\..\..\Qt\5.3\msvc2013

echo Copying DLL dependencies to %~dp0%BIN_PATH%...

REM >>> Libconfig
if not exist "%BIN_PATH%\libconfig++.dll" copy "%LIBCONFIG_PATH%\Debug\libconfig++.dll" "%BIN_PATH%\"
REM >>> Qt5 (prebuilt, dynamic)
if not exist "%BIN_PATH%\platforms" mkdir "%BIN_PATH%\platforms
if not exist "%BIN_PATH%\platforms\qwindowsd.dll" copy "%QT_PATH%\plugins\platforms\qwindowsd.dll" "%BIN_PATH%\platforms\"
if not exist "%BIN_PATH%\Qt5Cored.dll" copy "%QT_PATH%\bin\Qt5Cored.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\Qt5Guid.dll" copy "%QT_PATH%\bin\Qt5Guid.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\Qt5Widgetsd.dll" copy "%QT_PATH%\bin\Qt5Widgetsd.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\icuin52.dll" copy "%QT_PATH%\bin\icuin52.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\icuuc52.dll" copy "%QT_PATH%\bin\icuuc52.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\icudt52.dll" copy "%QT_PATH%\bin\icudt52.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\libEGLd.dll" copy "%QT_PATH%\bin\libEGLd.dll" "%BIN_PATH%\"
if not exist "%BIN_PATH%\libGLESv2d.dll" copy "%QT_PATH%\bin\libGLESv2d.dll" "%BIN_PATH%\"

popd
