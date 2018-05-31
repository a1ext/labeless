@goto skip_vcvars
@call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
:skip_vcvars
@set MSB="c:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe"
@set SLN=labeless.sln

::@goto ida
@echo building labeless for OllyDbg [1.1, 2.01]...
@call %MSB% %SLN% /t:labeless_olly:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err

@call %MSB% %SLN% /t:labeless_olly:Rebuild /p:Configuration=Release_DeFixed /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err
@call %MSB% %SLN% /t:labeless_olly2:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err


@echo building labeless for x64dbg...
@call %MSB% %SLN% /t:labeless_x64dbg:Rebuild /p:Configuration=Release /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err
@call %MSB% %SLN% /t:labeless_x64dbg:Rebuild /p:Configuration=Release /p:Platform=x64 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err

:ida
@set MOVE_USER_BACK=0
@IF EXIST labeless_ida\labeless_ida.vcxproj.user (
    @set MOVE_USER_BACK=1
    @move /Y labeless_ida\labeless_ida.vcxproj.user labeless_ida\labeless_ida.vcxproj.user_
)
@echo building labeless for IDA 6.8...
@call %MSB% %SLN% /t:labeless_ida:Rebuild /p:Configuration=IDA68_vc120 /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err
@call %MSB% %SLN% /t:labeless_ida:Rebuild /p:Configuration=IDA68_vc120_x64 /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err

@echo building labeless for IDA 6.9X...
@call %MSB% %SLN% /t:labeless_ida:Rebuild /p:Configuration=IDA69X /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err
@call %MSB% %SLN% /t:labeless_ida:Rebuild /p:Configuration=IDA69X_x64 /p:Platform=Win32 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err

@echo building labeless for IDA 7.0 x64...
@call %MSB% %SLN% /t:labeless_ida:Rebuild /p:Configuration=IDA7 /p:Platform=x64 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err
@call %MSB% %SLN% /t:labeless_ida:Rebuild /p:Configuration=IDA7_x64 /p:Platform=x64 /v:m
@IF /I "%ERRORLEVEL%" neq "0" goto err

@cd deploy
@call build_and_upload_pkg.bat c:\Python27
@cd ..

@echo All targets buit successfully, enjoy them :3

@goto end
:err:
@echo Build failed, exiting...
:end
@IF /I "%MOVE_USER_BACK%"=="1" (
    @move /Y labeless_ida\labeless_ida.vcxproj.user_ labeless_ida\labeless_ida.vcxproj.user
)