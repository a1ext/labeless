%module comment
%{
#include "./pluginsdk-swig/_plugins.h"
#include "./pluginsdk-swig/_scriptapi_comment.h"
%}

// Allow Python Buffers
%include <pybuffer.i>

%include "./pluginsdk-swig/_scriptapi_comment.h"
