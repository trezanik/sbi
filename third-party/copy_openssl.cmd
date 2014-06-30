@echo off
pushd %~dp0

REM /================================
REM / setup variables
REM /================================

set OPENSSL_VERSION=1.0.1h
set OPENSSL_PATH=..\..\openssl\%OPENSSL_VERSION%
set DEST_PATH=.\openssl
set LIB_PATH=%DEST_PATH%\lib

REM /================================
REM / wipe out old files
REM /================================
rd /s /q %DEST_PATH%

REM /================================
REM / copy inclusions
REM /================================

REM subfolder called openssl as all we need, so copy from parent
xcopy /E /H /I /Y /Q "%OPENSSL_PATH%\include" "%DEST_PATH%\"

REM /================================
REM / copy libraries
REM /================================

mkdir %LIB_PATH%

REM By default, there'll be a huge mass of libraries, binaries, pdbs, etc.
REM We only have need for two of these, so copy them manually
copy /Y "%OPENSSL_PATH%\out32dll\libeay32.lib" "%LIB_PATH%\"
copy /Y "%OPENSSL_PATH%\out32dll\ssleay32.lib" "%LIB_PATH%\"

popd
