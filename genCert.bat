@echo off
setlocal

echo "You must be administrator when you run this"

powershell.exe -executionpolicy bypass .\generateCert.ps1 || goto :fail

goto :end

:fail
echo error: powershell.exe failed %errorlevel%
exit /b %errorlevel%

:end
endlocal
