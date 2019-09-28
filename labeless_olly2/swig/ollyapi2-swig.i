%module ollyapi2
%{
    #include "..\sdk\plugin.h"
%}

// http://www.swig.org/Doc1.3/Library.html#Library_carrays
%include <carrays.i>
// http://www.swig.org/Doc1.3/Library.html#Library_nn7
%include <cdata.i>
%include <cpointer.i>
%pointer_functions(int, int_p);
%pointer_functions(ulong, ulong_p);


// http://www.swig.org/Doc1.3/Python.html#Python_nn75 && http://swig.10945.n7.nabble.com/Functions-writing-binary-data-into-buffer-td10714.html
// For Readmemory
%include <pybuffer.i>

// For types like HANDLE
%include <windows.i>

%include <pywstrings.swg>
%include <typemaps/cwstring.swg>

// From http://stackoverflow.com/questions/12686400/how-to-use-swig-to-treat-a-void-function-parameter-as-python-string
// Useful for Writememory
%typemap(in) const void* = const char*;
// Useful for Expression
%typemap(in) uchar* = char*;
%typemap(in) COLORREF = DWORD;

// From http://www.swig.org/Doc1.3/Library.html#Library_carrays
%array_class(ulong, ulongArray);
%array_class(t_asmmod, t_asmmodArray);
%array_class(t_secthdr, t_secthdrArray);
%array_class(t_operand, t_operandArray);
%array_class(t_metadata, t_metadataArray);
%array_class(t_netstream, t_netstreamArray);
%array_class(t_scheme, t_schemeArray);
%array_class(t_font, t_fontArray);
%array_class(COLORREF, COLORREFArray);
%array_class(t_memfield, t_memfieldArray);
%array_class(t_opinfo, t_opinfoArray);
%array_class(t_modop, t_modopArray);
%array_class(t_histrec, t_histrecArray);
%array_class(t_range, t_rangeArray);
%array_class(t_bincmd, t_bincmdArray);
#define HANDLE ulong
%feature("autodoc", "1");
%include "..\sdk\plugin-swig.h"