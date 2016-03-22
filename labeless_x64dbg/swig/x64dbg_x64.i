%module x64dbgapi64
#define _WIN64
%feature("autodoc", "1");
%include <carrays.i>
%include <cdata.i>
%include <cpointer.i>
%include <windows.i>

// For Readmemory
%include <pybuffer.i>

%include <pywstrings.swg>
%include <typemaps/cwstring.swg>

// bridgemain.h
%include "bridgemain.i"

// _plugins.h
%include "_plugins.i"

// _scriptapi*.h
%include "script.i"

%pointer_functions(int, intp);
%pointer_functions(unsigned int, uintp);
%pointer_functions(duint, duintp);
