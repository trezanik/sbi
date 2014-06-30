@echo off
pushd %~dp0

REM /================================
REM / setup variables
REM /================================

set LIBCONFIG_VERSION=1.4.9
set LIBCONFIG_PATH=..\..\libconfig\%LIBCONFIG_VERSION%
set DEST_PATH=.\libconfig

REM /================================
REM / wipe out old files
REM /================================
rd /s /q %DEST_PATH%

REM /================================
REM / copy inclusions
REM /================================

REM Sadly, libconfig stored project files and other things in the same folder
REM as the headers and the source, Wildcard the header files in as a fix.
xcopy /H /I /Y /Q "%LIBCONFIG_PATH%\lib\*.h" "%DEST_PATH%\libconfig\"
xcopy /H /I /Y /Q "%LIBCONFIG_PATH%\lib\*.hh" "%DEST_PATH%\libconfig\"
xcopy /H /I /Y /Q "%LIBCONFIG_PATH%\lib\*.h++" "%DEST_PATH%\libconfig\"

REM /================================
REM / copy libraries
REM /================================

mkdir %DEST_PATH%\lib
if exist "%LIBCONFIG_PATH%\Debug" copy /Y /Q "%LIBCONFIG_PATH%\Debug\libconfig++.lib" "%DEST_PATH%\lib\Debug\"
if exist "%LIBCONFIG_PATH%\Release" copy /Y /Q "%LIBCONFIG_PATH%\Release\libconfig++.lib" "%DEST_PATH%\lib\Release\"


popd
