@echo off
SETLOCAL EnableDelayedExpansion

set PYTHON_DIR=%1
if "%PYTHON_DIR%"=="" (
    echo [Error] Invalid number of arguments. Usage: build_sdist.bat "python_dir"
    goto :eof
)

set PYTHON_INTERP=%PYTHON_DIR%\python.exe
set PYTHON_SCRIPTS_DIR=%PYTHON_DIR%\Scripts
set TWINE_PATH=%PYTHON_SCRIPTS_DIR%\twine.exe
shift

%PYTHON_INTERP% setup.py sdist --formats=zip
if %ERRORLEVEL% == 0 (
	echo [OK] the package built succesfully
) else (
	echo [Failed] Unable to build the sdist, err code: %ERRORLEVEL%
	goto end
)
goto end
%TWINE_PATH% upload -r pypitest dist\*
if %ERRORLEVEL% == 0 (
	echo [OK] succesfully uploaded
) else (
	echo [Failed] Unable to upload to pypitest
	goto end
)

:end


