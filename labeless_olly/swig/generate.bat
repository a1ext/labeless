@echo off
..\..\3rdparty\swigwin-3.0.2\swig.exe -modern -python -I. ollyapi.i
@copy /Y .\ollyapi.py ..\..\deploy\labeless\backend\ollydbg11
echo Also need to patch generated .c file:
echo    _wrap_Readmemory()
pause