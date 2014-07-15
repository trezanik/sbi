@echo off
REM We want to support direct network paths with spaces
REM Doxygen config uses '../src' as the source directory, so we need to be able
REM to work with relative paths; pushd & popd accomplish this
pushd "%~dp0"
set DOXYGEN_VER=1.8.5

doxygen-%DOXYGEN_VER%.exe sbi.win32.doxygen > doxygen.log

REM Replace the default css with our own
copy doxygen-%DOXYGEN_VER%-custom.css html\doxygen.css > nul

popd
echo.
echo >>> Finished
echo.
pause > nul