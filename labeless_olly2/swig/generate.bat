@echo off
..\..\3rdparty\swigwin-3.0.2\swig.exe -modern -cpperraswarn -python -I. -o ollyapi2_wrap.c ollyapi2-swig.i
c:\windows\python27\python.exe correct_cdecl_fpointers.py ollyapi2_wrap.c
@copy /Y .\ollyapi2.py ..\..\test2\labeless_ollyapi\
pause