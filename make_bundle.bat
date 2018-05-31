@echo off
set CURR_DIR=%~dp0
set CURR_DIR=%CURR_DIR:~0,-1%
set VERSION_FILE_PATH=%CURR_DIR%\deploy\labeless\VERSION
set /p VERSION= <"%VERSION_FILE_PATH%"
set RELEASE_DIR=%CURR_DIR%\labeless_release_full_%VERSION%
set BIN_DIR=%CURR_DIR%\bin

mkdir %RELEASE_DIR%

echo DeFixed...
set DE_FIXED_OUT_DIR=%RELEASE_DIR%\DeFixed110\plugins\
mkdir "%DE_FIXED_OUT_DIR%"
xcopy /Y /F "%BIN_DIR%\labeless_olly_foff.dll" %DE_FIXED_OUT_DIR%
if %ERRORLEVEL% neq 0 (
	echo [Failed] Unable to bundle DeFixed
	goto end
)

echo Olly 1.10...
set OLLY1_OUT_DIR=%RELEASE_DIR%\OllyDbg110\
set OLLY1_SRC_DIR=%CURR_DIR%\test\
pushd "%OLLY1_SRC_DIR%"
for /f "tokens=*" %%G IN ('git ls-files') do (
	echo %%G | findstr /C:.gitignore > nul || echo F | xcopy /Y /F "%OLLY1_SRC_DIR%\%%G" "%OLLY1_OUT_DIR%\%%G"
)
xcopy /Y /F "%BIN_DIR%\labeless_olly.dll" "%OLLY1_OUT_DIR%\plugins\"
popd

echo Olly 2.01...
set OLLY2_OUT_DIR=%RELEASE_DIR%\OllyDbg201\
set OLLY2_SRC_DIR=%CURR_DIR%\test2\
pushd "%OLLY2_SRC_DIR%"
for /f "tokens=*" %%G IN ('git ls-files') do (
	echo %%G | findstr /C:.gitignore > nul || echo F | xcopy /Y /F "%OLLY2_SRC_DIR%\%%G" "%OLLY2_OUT_DIR%\%%G"
)
xcopy /Y /F "%BIN_DIR%\labeless_olly2.dll" "%OLLY2_OUT_DIR%\plugins\"
popd

echo x64dbg...
set X64DBG_OUT_DIR=%RELEASE_DIR%\x64dbg\
set X64DBG_SRC_DIR=%CURR_DIR%\test_x64dbg\

pushd %X64DBG_SRC_DIR%
for /f "tokens=*" %%G in ('git ls-files') do (
	echo %%G | findstr /C:.gitignore > nul || echo F | xcopy /Y /F "%X64DBG_SRC_DIR%\%%G" "%X64DBG_OUT_DIR%\%%G"
)
popd

xcopy /Y /F "%BIN_DIR%\labeless_x64dbg.dp32" "%X64DBG_OUT_DIR%\x32\plugins\"
xcopy /Y /F "%BIN_DIR%\labeless_x64dbg.dp64" "%X64DBG_OUT_DIR%\x64\plugins\"

echo IDA68...
set IDA68_OUT_DIR=%RELEASE_DIR%\IDA68\plugins\
xcopy /Y /F "%BIN_DIR%\labeless_ida_68.plw" "%IDA68_OUT_DIR%"
xcopy /Y /F "%BIN_DIR%\labeless_ida_68.p64" "%IDA68_OUT_DIR%"

echo IDA69X...
set IDA69X_OUT_DIR=%RELEASE_DIR%\IDA69\plugins\
xcopy /Y /F "%BIN_DIR%\labeless_ida_690.plw" "%IDA69X_OUT_DIR%"
xcopy /Y /F "%BIN_DIR%\labeless_ida_690.p64" "%IDA69X_OUT_DIR%"
if exist "%BIN_DIR%\labeless_ida_690.plx" xcopy /Y /F "%BIN_DIR%\labeless_ida_690.plx" "%IDA69X_OUT_DIR%"
if exist "%BIN_DIR%\labeless_ida_690.plx64" xcopy /Y /F "%BIN_DIR%\labeless_ida_690.plx64" "%IDA69X_OUT_DIR%"
if exist "%BIN_DIR%\libprotobuf.so.9" xcopy /Y /F "%BIN_DIR%\libprotobuf.so.9" "%IDA69X_OUT_DIR%\..\"

echo IDA7X...
set IDA7X_OUT_DIR=%RELEASE_DIR%\IDA7X\plugins\
xcopy /Y /F "%BIN_DIR%\labeless_ida_70.dll" "%IDA7X_OUT_DIR%"
xcopy /Y /F "%BIN_DIR%\labeless_ida_70_64.dll" "%IDA7X_OUT_DIR%"

echo README...
xcopy /Y /F "%CURR_DIR%\README.md" "%RELEASE_DIR%\"

echo Python package...
set DEPLOY_OUT_DIR=%RELEASE_DIR%\deploy\
set DEPLOY_SRC_DIR=%CURR_DIR%\deploy\
mkdir "%DEPLOY_OUT_DIR%"
xcopy /Y /F "%DEPLOY_SRC_DIR%\dist\labeless-%VERSION%-py2.py3-none-any.whl" "%DEPLOY_OUT_DIR%\"
xcopy /Y /F "%DEPLOY_SRC_DIR%\protobuf-2.6.1-py2.7.egg" "%DEPLOY_OUT_DIR%\"
xcopy /Y /F "%DEPLOY_SRC_DIR%\deploy_labeless_to_vm.py" "%DEPLOY_OUT_DIR%\"
xcopy /Y /F "%DEPLOY_SRC_DIR%\deploy.conf" "%DEPLOY_OUT_DIR%\"
xcopy /Y /F "%DEPLOY_SRC_DIR%\get-pip.py" "%DEPLOY_OUT_DIR%\"
xcopy /Y /F "%DEPLOY_SRC_DIR%\setup_protobuf.py" "%DEPLOY_OUT_DIR%\"

echo docs...
set DOCS_OUT_DIR=%RELEASE_DIR%\docs\
set DOCS_SRC_DIR=%CURR_DIR%\docs\
pushd "%DOCS_SRC_DIR%"
for /f "tokens=*" %%G in ('git ls-files --exclude-standard') do (
	echo %%G | findstr /C:.gitignore > nul || echo F | xcopy /Y /F "%DOCS_SRC_DIR%\%%G" "%DOCS_OUT_DIR%\%%G"
)
popd

:end
