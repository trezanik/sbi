REM Generate uic, moc files
call ../../src/Qt5GUI/manual_generate_formcode.cmd
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
REM Generate resource files
call ../../resources/manual_generate_resources.cmd
if %ERRORLEVEL% GEQ 1 exit /b %ERRORLEVEL%
pause > nul