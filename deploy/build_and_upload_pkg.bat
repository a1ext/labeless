@echo off
::SETLOCAL EnableDelayedExpansion

set PYTHON_DIR=%1
if "%PYTHON_DIR%"=="" (
    echo [Error] Invalid number of arguments. Usage: build_sdist.bat "python_dir"
    goto :eof
)

set PYTHON_INTERP=%PYTHON_DIR%\python.exe
set PYTHON_SCRIPTS_DIR=%PYTHON_DIR%\Scripts
set TWINE_PATH=%PYTHON_SCRIPTS_DIR%\twine.exe
set CURR_DIR=%~dp0
set CURR_DIR=%CURR_DIR:~0,-1%
set DIST_DIR=%CURR_DIR%\dist\
set BUILD_DIR=%CURR_DIR%\build\
shift

if exist "%DIST_DIR%" (
	echo [CLEAN] Removing directory "%DIST_DIR%"...
	rd /q /s "%DIST_DIR%"
)
if exist "%BUILD_DIR%" (
	echo [CLEAN] Removing directory "%BUILD_DIR%"...
	rd /q /s "%BUILD_DIR%"
)


%PYTHON_INTERP% setup.py bdist_wheel
if %ERRORLEVEL% == 0 (
	echo [OK] the package built succesfully
) else (
	echo [Failed] Unable to build the bdist_wheel, err code: %ERRORLEVEL%
	goto end
)

::goto skip_upload
:: upload to test.pypi.org
%TWINE_PATH% upload --repository-url https://test.pypi.org/legacy/ dist\*.whl
if %ERRORLEVEL% == 0 (
	echo [OK] succesfully uploaded
) else (
	echo [Failed] Unable to upload to test.pypi.org
	goto end
)
:skip_upload
set VERSION_FILE_PATH=%CURR_DIR%\labeless\VERSION
set /p VERSION= <"%VERSION_FILE_PATH%"
echo version: %VERSION%

set TMP_DIR=%CURR_DIR%\tmp\
if not exist "%TMP_DIR%" mkdir "%TMP_DIR%"
cd "%TMP_DIR%"
:: test the uploaded file
%PYTHON_INTERP% -m pip install --index-url https://test.pypi.org/simple/ --upgrade --no-deps --ignore-installed labeless==%VERSION%
if %ERRORLEVEL% == 0 (
	echo [OK] installed
) else (
	echo [Failed] Unable to install uploaded package
	goto end
)
cd "%CURR_DIR%"
rd /q /s "%TMP_DIR%"
echo Finished
:end


