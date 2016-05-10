%module script
%{
#include "../pluginsdk/_scriptapi.h"
#include "../pluginsdk/_scriptapi_assembler.h"
#include "../pluginsdk/_scriptapi_bookmark.h"
#include "../pluginsdk/_scriptapi_comment.h"
#include "../pluginsdk/_scriptapi_debug.h"
#include "../pluginsdk/_scriptapi_flag.h"
#include "../pluginsdk/_scriptapi_function.h"
#include "../pluginsdk/_scriptapi_gui.h"
#include "../pluginsdk/_scriptapi_label.h"
#include "../pluginsdk/_scriptapi_memory.h"
#include "../pluginsdk/_scriptapi_misc.h"
#include "../pluginsdk/_scriptapi_module.h"
#include "../pluginsdk/_scriptapi_pattern.h"
#include "../pluginsdk/_scriptapi_register.h"
#include "../pluginsdk/_scriptapi_stack.h"
#include "../pluginsdk/_scriptapi_symbol.h"
%}

%include "./pluginsdk-swig/scriptapi_swig.h"


%array_class(Script::Bookmark::BookmarkInfo, BookmarkInfoArray);
%array_class(Script::Comment::CommentInfo, CommentInfoArray);
%array_class(Script::Function::FunctionInfo, FunctionInfoArray);
%array_class(Script::Label::LabelInfo, LabelInfoArray);
%array_class(Script::Module::ModuleInfo, ModuleInfoArray);
%array_class(Script::Module::ModuleSectionInfo, ModuleSectionInfoArray);
%array_class(Script::Symbol::SymbolInfo, SymbolInfoArray);

