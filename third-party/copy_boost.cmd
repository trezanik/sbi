@echo off
pushd %~dp0

REM /================================
REM / setup variables
REM /================================

set BOOST_VERSION=1_55_0
set BOOST_PATH=..\..\boost\%BOOST_VERSION%
set DEST_PATH=.\boost
set LIB_PATH=%DEST_PATH%\lib
set DATE_TIME_PATH=%BOOST_PATH%\lib32-msvc-12.0\libboost_date_time-vc120
set SYSTEM_PATH=%BOOST_PATH%\lib32-msvc-12.0\libboost_system-vc120
set REGEX_PATH=%BOOST_PATH%\lib32-msvc-12.0\libboost_regex-vc120

REM /================================
REM / wipe out old files
REM /================================
echo Removing old files...
REM rd /s /q %DEST_PATH%

REM /================================
REM / copy inclusions
REM /================================
echo Copying inclusions...
REM xcopy /E /H /I /Y /Q "%BOOST_PATH%\boost" "%DEST_PATH%\boost"

REM /================================
REM / copy libraries
REM /================================

mkdir "%LIB_PATH%\Release"
mkdir "%LIB_PATH%\Debug"

echo Copying libraries...
REM Copy only the libraries we specifically need, as altogether these are massive
copy /Y "%DATE_TIME_PATH%-mt-gd-1_55.lib" "%LIB_PATH%\Debug\"
copy /Y "%DATE_TIME_PATH%-gd-1_55.lib" "%LIB_PATH%\Debug\"
copy /Y "%DATE_TIME_PATH%-mt-1_55.lib" "%LIB_PATH%\Release\"
copy /Y "%DATE_TIME_PATH%-1_55.lib" "%LIB_PATH%\Release\"
copy /Y "%SYSTEM_PATH%-mt-gd-1_55.lib" "%LIB_PATH%\Debug\"
copy /Y "%SYSTEM_PATH%-gd-1_55.lib" "%LIB_PATH%\Debug\"
copy /Y "%SYSTEM_PATH%-mt-1_55.lib" "%LIB_PATH%\Release\"
copy /Y "%SYSTEM_PATH%-1_55.lib" "%LIB_PATH%\Release\"
copy /Y "%REGEX_PATH%-mt-gd-1_55.lib" "%LIB_PATH%\Debug\"
copy /Y "%REGEX_PATH%-gd-1_55.lib" "%LIB_PATH%\Debug\"
copy /Y "%REGEX_PATH%-mt-1_55.lib" "%LIB_PATH%\Release\"
copy /Y "%REGEX_PATH%-1_55.lib" "%LIB_PATH%\Release\"

popd
