@echo off
..\..\3rdparty\swigwin-3.0.2\swig.exe -c++ -outdir . -Wall -modern -o __x64dbg.cpp -cpperraswarn -python -v -debug-symbols x64dbg.i 
set ERROR_X86=%ERRORLEVEL%
..\..\3rdparty\swigwin-3.0.2\swig.exe -c++ -outdir . -Wall -modern -o __x64dbg_x64.cpp -cpperraswarn -python -v -debug-symbols x64dbg_x64.i 
if %ERRORLEVEL% == 0 (
    echo [OK] generated.
    @xcopy .\x64dbgapi.py ..\..\deploy\labeless\backend\x64dbg\x64dbgapi.py /s /e /Y
    @xcopy .\x64dbgapi64.py ..\..\deploy\labeless\backend\x64dbg\x64dbgapi64.py /s /e /Y
    echo Done
) else (
    echo Error occurred! check the log
)
pause