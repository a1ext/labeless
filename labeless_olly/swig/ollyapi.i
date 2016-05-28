%module ollyapi
%{
#include <windows.h>
#include "../sdk/plugin.h"
%}

%include <carrays.i>
%include <cdata.i>
%include <cpointer.i>

// For Readmemory
%include <pybuffer.i>

// For types like HANDLE
%include <windows.i>

%include <pywstrings.swg>
%include <typemaps/cwstring.swg>

// Useful for Writememory
%typemap(in) const void* = const char*;
// Useful for Expression
%typemap(in) uchar* = char*;

%array_class(ulong, ulongArray);


%include "../sdk/plugin-swig.h"

