%module assembler
%{
#include "./pluginsdk-swig/_plugins.h"
#include "./pluginsdk-swig/_scriptapi_assembler.h"
%}

// Allow Python Buffers
%include <pybuffer.i>

%include "./pluginsdk-swig/_scriptapi_assembler.h"
