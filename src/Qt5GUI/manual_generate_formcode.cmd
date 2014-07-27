@echo off
REM ##################################################################################
REM # Manually executes the equivalent of what qmake does; should be handy if needed #
REM ##################################################################################
set CURRENT_PATH=%~dp0
pushd "%CURRENT_PATH%"

REM Adjust as needed
set QT_PATH=..\..\..\Qt\5.3\msvc2013\bin
set FORMS_PATH=.\forms
set GEN_PATH=.\generated

if not exist "%GEN_PATH%" mkdir "%GEN_PATH%"

for %%f in (%FORMS_PATH%\*.ui) do (
	echo Executing UIC and MOC for %%f..
	REM Generate the header
	%QT_PATH%\uic.exe %%f -o %GEN_PATH%\ui_%%~nf.h
	REM Generate the moc
	%QT_PATH%\moc.exe %%~nf.h -o %GEN_PATH%\moc_%%~nf.cc
)

REM non-form files that require moc
set NONUI_FILE=UiThreadExec
echo Executing MOC for %NONUI_FILE%.h..
%QT_PATH%\moc.exe %NONUI_FILE%.h -o %GEN_PATH%\moc_%NONUI_FILE%.cc

popd
echo.
echo ### Finished
echo.
