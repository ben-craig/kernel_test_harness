@echo off
setlocal

if not exist generated_targets_from_gen.rb.ninja ( ruby gen.rb || goto :gen_fail)

ninja %* || goto :ninja_fail

goto :end

:gen_fail
echo error: gen.rb failed %errorlevel%
exit /b %errorlevel%

:ninja_fail
echo error: ninja failed %errorlevel%
exit /b %errorlevel%

:end
endlocal
