@echo off
REM ########################################################################
REM # Manually creates a source file containing the resources from the qrc #
REM ########################################################################
set CURRENT_PATH=%~dp0
pushd "%CURRENT_PATH%"


set QT_PATH=..\..\Qt\5.3\msvc2013_64\bin
set GEN_PATH=.\generated

if not exist "%GEN_PATH%" mkdir "%GEN_PATH%"

echo Generating source via Resource Compiler..
%QT_PATH%\rcc.exe -name core .\sbi.qrc -o %GEN_PATH%\qrc_sbi.cc


popd
echo.
echo ### Finished
echo.
