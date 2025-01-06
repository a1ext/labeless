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
%include "cwstring.i"

// For types like HANDLE
%include <windows.i>

%include <pywstrings.swg>
%include <typemaps/cwstring.swg>

// From http://stackoverflow.com/questions/12686400/how-to-use-swig-to-treat-a-void-function-parameter-as-python-string
// Useful for Writememory
%typemap(in) const void* = const char*;
// Useful for Expression
%typemap(in) uchar* = char*;
//%typemap(in) COLORREF = DWORD;

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
%array_class(t_argdec, t_argdecArray);
%array_class(t_strdec, t_strdecArray);
#define HANDLE void *
%feature("autodoc", "1");


//%clear wchar_t*;
// Typemap for wchar_t* -> Python string conversion
//%typemap(in) wchar_t* {
//    if (PyObject_TypeCheck($input, &PyUnicode_Type)) {
//        wchar_t *temp = PyUnicode_AsWideCharString($input, NULL);
//        if (!temp) {
//            SWIG_exception_fail(SWIG_TypeError, "Failed to convert Python string to wchar_t*");
//        }
//        $1 = temp;
//    } else if (PyObject_TypeCheck($input, &PyCapsule_Type)) {
//        $1 = (wchar_t*)PyCapsule_GetPointer($input, NULL);
//    } else {
//        SWIG_exception_fail(SWIG_TypeError, "Expected a string, None, or ctypes buffer");
//    }
//}

%typemap(out) wchar_t* {
    if ($1 == NULL) {
        $result = Py_None;
        Py_INCREF($result);
    } else {
        $result = PyUnicode_FromWideChar($1, wcslen($1));
    }
}

%define %pybuffer_mutable_string_optional(TYPEMAP)
%typemap(in) (TYPEMAP)
  (int res, Py_ssize_t size = 0, void *buf = 0) {
  if ($input == Py_None) {
    $1 = ($1_ltype) NULL;
  }
  else {
    res = PyObject_AsWriteBuffer($input, &buf, &size);
    if (res<0) {
      PyErr_Clear();
      %argument_fail(res, "(TYPEMAP, SIZE)", $symname, $argnum);
    }
    $1 = ($1_ltype) buf;
  }
}
%enddef

%define %pointer_cast_cpp(TYPE1,TYPE2,NAME)
%inline %{
TYPE2 NAME(TYPE1 x) {
   return %reinterpret_cast(x, TYPE2);
}
%}
%enddef


%include "..\sdk\plugin-swig.h"