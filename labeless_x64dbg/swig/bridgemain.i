%module bridgemain
%{
#include "bridgelist.h"
#include "bridgemain.h"
#include "_dbgfunctions.h"
%}

// Allow Python Buffers
%include <pybuffer.i>
%include <cpointer.i>

// Type Maps
/* Convert from C --> Python */
%typemap(out) HWND {
    $result = PyInt_FromLong((long)$1);
}

%include "pluginsdk-swig/bridgelist.h"
%include "pluginsdk-swig/bridgemain_swig.h"
%array_class(MEMPAGE, MEMPAGEArray);
%pointer_cast(void*, MEMPAGE*, void_to_MEMPAGE);
%include "pluginsdk-swig/_dbgfunctions.h"
//%pointer_class(DBGFUNCTIONS, DBGFUNCTIONSPtr);


%extend DBGFUNCTIONS_
{
    %pybuffer_mutable_string(char* error)
    bool AssembleAtEx_(duint addr, const char* instruction, char* error, bool fillnop)
    { return $self->AssembleAtEx(addr, instruction, error, fillnop); }
    %typemap(in) char* error;
    
    %pybuffer_mutable_string(char* section)
    bool SectionFromAddr_(duint addr, char* section)
    { return $self->SectionFromAddr(addr, section); }
    %typemap(in) char* section;
    
    %pybuffer_mutable_string(char* modname)
    bool ModNameFromAddr_(duint addr, char* modname, bool extension)
    { return $self->ModNameFromAddr(addr, modname, extension); }
    %typemap(in) char* modname;
    
    duint ModBaseFromAddr_(duint addr)
    { return $self->ModBaseFromAddr(addr); }
    
    duint ModBaseFromName_(const char* modname)
    { return $self->ModBaseFromName(modname); }
    
    duint ModSizeFromAddr_(duint addr)
    { return $self->ModSizeFromAddr(addr); }
    
    %pybuffer_mutable_string(char* error)
    bool Assemble_(duint addr, unsigned char* dest, int* size, const char* instruction, char* error)
    { return $self->Assemble(addr, dest, size, instruction, error); }
    %typemap(in) char* error;
    
    bool PatchGet_(duint addr)
    { return $self->PatchGet(addr); }
    
    bool PatchInRange_(duint start, duint end)
    { return $self->PatchInRange(start, end); }
    
    bool MemPatch_(duint va, const unsigned char* src, duint size)
    { return $self->MemPatch(va, src, size); }    
    
    void PatchRestoreRange_(duint start, duint end)
    { return $self->PatchRestoreRange(start, end); }
    
    bool PatchEnum_(DBGPATCHINFO* patchlist, size_t* cbsize)
    { return $self->PatchEnum(patchlist, cbsize); }
    
    bool PatchRestore_(duint addr)
    { return $self->PatchRestore(addr); }
    
    %pybuffer_mutable_string(char* error)
    int PatchFile_(DBGPATCHINFO* patchlist, int count, const char* szFileName, char* error)
    { return $self->PatchFile(patchlist, count, szFileName, error); }
    %typemap(in) char* error;
    
    %pybuffer_mutable_string(char* path)
    int ModPathFromAddr_(duint addr, char* path, int size)
    { return $self->ModPathFromAddr(addr, path, size); }
    %typemap(in) char* path;
    
    %pybuffer_mutable_string(char* path)
    int ModPathFromName_(const char* modname, char* path, int size)
    { return $self->ModPathFromName(modname, path, size); }
    %typemap(in) char* path;
    
    %pybuffer_mutable_string(const unsigned char* data)
    bool DisasmFast_(const unsigned char* data, duint addr, BASIC_INSTRUCTION_INFO* basicinfo)
    { return $self->DisasmFast(data, addr, basicinfo); }
    %typemap(in) const unsigned char* data;
    
    void MemUpdateMap_()
    { return $self->MemUpdateMap(); }
    
    void GetCallStack_(DBGCALLSTACK* callstack)
    { return $self->GetCallStack(callstack); }
    
    void SymbolDownloadAllSymbols_(const char* szSymbolStore)
    { return $self->SymbolDownloadAllSymbols(szSymbolStore); }
    
    bool GetJit_(char* jit, bool x64)
    { return $self->GetJit(jit, x64); }
    
    bool GetJitAuto_(bool* jitauto)
    { return $self->GetJitAuto(jitauto); }
    
    bool GetDefJit_(char* defjit)
    { return $self->GetDefJit(defjit); }
    
    bool GetProcessList_(DBGPROCESSINFO** entries, int* count)
    { return $self->GetProcessList(entries, count); }
    
    %pybuffer_mutable_string(char* rights)
    bool GetPageRights_(duint addr, char* rights)
    { return $self->GetPageRights(addr, rights); }
    %typemap(in) char* rights;
    
    bool SetPageRights_(duint addr, const char* rights)
    { return $self->SetPageRights(addr, rights); }
    
    %pybuffer_mutable_string(char* rights)
    bool PageRightsToString_(DWORD protect, char* rights)
    { return $self->PageRightsToString(protect, rights); }
    %typemap(in) char* rights;
    
    bool IsProcessElevated_()    
    { return $self->IsProcessElevated(); }
    
    %pybuffer_mutable_string(char* cmdline)
    bool GetCmdline_(char* cmdline, size_t* cbsize)
    { return $self->GetCmdline(cmdline, cbsize); }
    %typemap(in) char* cmdline;
    
    bool SetCmdline_(const char* cmdline)
    { return $self->SetCmdline(cmdline); }
    
    duint FileOffsetToVa_(const char* modname, duint offset)
    { return $self->FileOffsetToVa(modname, offset); }
    
    duint VaToFileOffset_(duint va)
    { return $self->VaToFileOffset(va); }
    
    duint GetAddrFromLine_(const char* szSourceFile, int line)
    { return $self->GetAddrFromLine(szSourceFile, line); }
    
    %pybuffer_mutable_string(char* szSourceFile)
    bool GetSourceFromAddr_(duint addr, char* szSourceFile, int* line)
    { return $self->GetSourceFromAddr(addr, szSourceFile, line); }
    %typemap(in) char* szSourceFile;
    
    bool ValFromString_(const char* string, duint* value)
    { return $self->ValFromString(string, value); }
    
    bool PatchGetEx_(duint addr, DBGPATCHINFO* info)
    { return $self->PatchGetEx(addr, info); }
    
}

