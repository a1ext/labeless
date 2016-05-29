#ifndef _BRIDGEMAIN_H_
#define _BRIDGEMAIN_H_

#include <windows.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

//list structure (and C++ wrapper)
#include "bridgelist.h"

//default structure alignments forced
#ifdef _WIN64
#pragma pack(push, 16)
#else //x86
#pragma pack(push, 8)
#endif //_WIN64

#ifdef _WIN64
typedef unsigned long long duint;
typedef signed long long dsint;
#else
typedef unsigned long duint;
typedef signed long dsint;
#endif //_WIN64


#ifdef __cplusplus
extern "C"
{
#endif

//Bridge defines
#define MAX_SETTING_SIZE 65536
#define DBG_VERSION 25

//Bridge functions
extern const char* BridgeInit();
extern const char* BridgeStart();
extern void* BridgeAlloc(size_t size);
extern void BridgeFree(void* ptr);

%pybuffer_string(char* value)
extern bool BridgeSettingGet(const char* section, const char* key, char* value);
extern bool BridgeSettingGetUint(const char* section, const char* key, duint* value);
extern bool BridgeSettingSet(const char* section, const char* key, const char* value);
extern bool BridgeSettingSetUint(const char* section, const char* key, duint value);
extern bool BridgeSettingFlush();
extern bool BridgeSettingRead(int* errorLine);
extern int BridgeGetDbgVersion();

//Debugger defines
#define MAX_LABEL_SIZE 256
#define MAX_COMMENT_SIZE 512
#define MAX_MODULE_SIZE 256
#define MAX_IMPORT_SIZE 65536
#define MAX_BREAKPOINT_SIZE 256
#define MAX_CONDITIONAL_EXPR_SIZE 256
#define MAX_CONDITIONAL_TEXT_SIZE 256
#define MAX_SCRIPT_LINE_SIZE 2048
#define MAX_THREAD_NAME_SIZE 256
#define MAX_STRING_SIZE 512
#define MAX_ERROR_SIZE 512
#define RIGHTS_STRING_SIZE (sizeof("ERWCG") + 1)
#define MAX_SECTION_SIZE 10
#define MAX_COMMAND_LINE_SIZE 256
#define MAX_MNEMONIC_SIZE 64
#define PAGE_SIZE 0x1000

//Debugger enums
typedef enum
{
    initialized,
    paused,
    running,
    stopped
} DBGSTATE;

typedef enum
{
    SEG_DEFAULT,
    SEG_ES,
    SEG_DS,
    SEG_FS,
    SEG_GS,
    SEG_CS,
    SEG_SS
} SEGMENTREG;

typedef enum
{
    flagmodule = 1,
    flaglabel = 2,
    flagcomment = 4,
    flagbookmark = 8,
    flagfunction = 16,
    flagloop = 32
} ADDRINFOFLAGS;

typedef enum
{
    bp_none = 0,
    bp_normal = 1,
    bp_hardware = 2,
    bp_memory = 4
} BPXTYPE;

typedef enum
{
    FUNC_NONE,
    FUNC_BEGIN,
    FUNC_MIDDLE,
    FUNC_END,
    FUNC_SINGLE
} FUNCTYPE;

typedef enum
{
    LOOP_NONE,
    LOOP_BEGIN,
    LOOP_MIDDLE,
    LOOP_ENTRY,
    LOOP_END
} LOOPTYPE;

typedef enum
{
    ARG_NONE,
    ARG_BEGIN,
    ARG_MIDDLE,
    ARG_END
} ARGTYPE;

typedef enum
{
    DBG_SCRIPT_LOAD,                // param1=const char* filename,      param2=unused
    DBG_SCRIPT_UNLOAD,              // param1=unused,                    param2=unused
    DBG_SCRIPT_RUN,                 // param1=int destline,              param2=unused
    DBG_SCRIPT_STEP,                // param1=unused,                    param2=unused
    DBG_SCRIPT_BPTOGGLE,            // param1=int line,                  param2=unused
    DBG_SCRIPT_BPGET,               // param1=int line,                  param2=unused
    DBG_SCRIPT_CMDEXEC,             // param1=const char* command,       param2=unused
    DBG_SCRIPT_ABORT,               // param1=unused,                    param2=unused
    DBG_SCRIPT_GETLINETYPE,         // param1=int line,                  param2=unused
    DBG_SCRIPT_SETIP,               // param1=int line,                  param2=unused
    DBG_SCRIPT_GETBRANCHINFO,       // param1=int line,                  param2=SCRIPTBRANCH* info
    DBG_SYMBOL_ENUM,                // param1=SYMBOLCBINFO* cbInfo,      param2=unused
    DBG_ASSEMBLE_AT,                // param1=duint addr,                param2=const char* instruction
    DBG_MODBASE_FROM_NAME,          // param1=const char* modname,       param2=unused
    DBG_DISASM_AT,                  // param1=duint addr,                 param2=DISASM_INSTR* instr
    DBG_STACK_COMMENT_GET,          // param1=duint addr,                param2=STACK_COMMENT* comment
    DBG_GET_THREAD_LIST,            // param1=THREADALLINFO* list,       param2=unused
    DBG_SETTINGS_UPDATED,           // param1=unused,                    param2=unused
    DBG_DISASM_FAST_AT,             // param1=duint addr,                param2=BASIC_INSTRUCTION_INFO* basicinfo
    DBG_MENU_ENTRY_CLICKED,         // param1=int hEntry,                param2=unused
    DBG_FUNCTION_GET,               // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_FUNCTION_OVERLAPS,          // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_FUNCTION_ADD,               // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_FUNCTION_DEL,               // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_LOOP_GET,                   // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_LOOP_OVERLAPS,              // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_LOOP_ADD,                   // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_LOOP_DEL,                   // param1=FUNCTION_LOOP_INFO* info,  param2=unused
    DBG_IS_RUN_LOCKED,              // param1=unused,                    param2=unused
    DBG_IS_BP_DISABLED,             // param1=duint addr,                param2=unused
    DBG_SET_AUTO_COMMENT_AT,        // param1=duint addr,                param2=const char* text
    DBG_DELETE_AUTO_COMMENT_RANGE,  // param1=duint start,               param2=duint end
    DBG_SET_AUTO_LABEL_AT,          // param1=duint addr,                param2=const char* text
    DBG_DELETE_AUTO_LABEL_RANGE,    // param1=duint start,               param2=duint end
    DBG_SET_AUTO_BOOKMARK_AT,       // param1=duint addr,                param2=const char* text
    DBG_DELETE_AUTO_BOOKMARK_RANGE, // param1=duint start,               param2=duint end
    DBG_SET_AUTO_FUNCTION_AT,       // param1=duint addr,                param2=const char* text
    DBG_DELETE_AUTO_FUNCTION_RANGE, // param1=duint start,               param2=duint end
    DBG_GET_STRING_AT,              // param1=duint addr,                param2=unused
    DBG_GET_FUNCTIONS,              // param1=unused,                    param2=unused
    DBG_WIN_EVENT,                  // param1=MSG* message,              param2=long* result
    DBG_WIN_EVENT_GLOBAL,           // param1=MSG* message,              param2=unused
    DBG_INITIALIZE_LOCKS,           // param1=unused,                    param2=unused
    DBG_DEINITIALIZE_LOCKS,         // param1=unused,                    param2=unused
    DBG_GET_TIME_WASTED_COUNTER,    // param1=unused,                    param2=unused
    DBG_SYMBOL_ENUM_FROMCACHE,      // param1=SYMBOLCBINFO* cbInfo,      param2=unused
    DBG_DELETE_COMMENT_RANGE,       // param1=duint start,               param2=duint end
    DBG_DELETE_LABEL_RANGE,         // param1=duint start,               param2=duint end
    DBG_DELETE_BOOKMARK_RANGE,      // param1=duint start,               param2=duint end
} DBGMSG;

typedef enum
{
    linecommand,
    linebranch,
    linelabel,
    linecomment,
    lineempty,
} SCRIPTLINETYPE;

typedef enum
{
    scriptnobranch,
    scriptjmp,
    scriptjnejnz,
    scriptjejz,
    scriptjbjl,
    scriptjajg,
    scriptjbejle,
    scriptjaejge,
    scriptcall
} SCRIPTBRANCHTYPE;

typedef enum
{
    instr_normal,
    instr_branch,
    instr_stack
} DISASM_INSTRTYPE;

typedef enum
{
    arg_normal,
    arg_memory
} DISASM_ARGTYPE;

typedef enum
{
    str_none,
    str_ascii,
    str_unicode
} STRING_TYPE;

typedef enum
{
    _PriorityIdle = -15,
    _PriorityAboveNormal = 1,
    _PriorityBelowNormal = -1,
    _PriorityHighest = 2,
    _PriorityLowest = -2,
    _PriorityNormal = 0,
    _PriorityTimeCritical = 15,
    _PriorityUnknown = 0x7FFFFFFF
} THREADPRIORITY;

typedef enum
{
    _Executive = 0,
    _FreePage = 1,
    _PageIn = 2,
    _PoolAllocation = 3,
    _DelayExecution = 4,
    _Suspended = 5,
    _UserRequest = 6,
    _WrExecutive = 7,
    _WrFreePage = 8,
    _WrPageIn = 9,
    _WrPoolAllocation = 10,
    _WrDelayExecution = 11,
    _WrSuspended = 12,
    _WrUserRequest = 13,
    _WrEventPair = 14,
    _WrQueue = 15,
    _WrLpcReceive = 16,
    _WrLpcReply = 17,
    _WrVirtualMemory = 18,
    _WrPageOut = 19,
    _WrRendezvous = 20,
    _Spare2 = 21,
    _Spare3 = 22,
    _Spare4 = 23,
    _Spare5 = 24,
    _WrCalloutStack = 25,
    _WrKernel = 26,
    _WrResource = 27,
    _WrPushLock = 28,
    _WrMutex = 29,
    _WrQuantumEnd = 30,
    _WrDispatchInt = 31,
    _WrPreempted = 32,
    _WrYieldExecution = 33,
    _WrFastMutex = 34,
    _WrGuardedMutex = 35,
    _WrRundown = 36,
} THREADWAITREASON;

typedef enum
{
    size_byte = 1,
    size_word = 2,
    size_dword = 4,
    size_qword = 8
} MEMORY_SIZE;

//Debugger typedefs
typedef MEMORY_SIZE VALUE_SIZE;
typedef struct SYMBOLINFO_ SYMBOLINFO;
#include "_dbgfunctions.h"

typedef void (*CBSYMBOLENUM)(SYMBOLINFO* symbol, void* user);

typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    duint RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

//Debugger structs
typedef struct
{
    MEMORY_BASIC_INFORMATION mbi;
    char info[MAX_MODULE_SIZE];
} MEMPAGE;


typedef struct
{
    int count;
    MEMPAGE* page;
    //void* page;
} MEMMAP;

typedef struct
{
    BPXTYPE type;
    duint addr;
    bool enabled;
    bool singleshoot;
    bool active;
    char name[MAX_BREAKPOINT_SIZE];
    char mod[MAX_MODULE_SIZE];
    unsigned short slot;
    // extended part
    unsigned int hitCount;
    bool fastResume;
    char breakCondition[MAX_CONDITIONAL_EXPR_SIZE];
    char logText[MAX_CONDITIONAL_TEXT_SIZE];
    char logCondition[MAX_CONDITIONAL_EXPR_SIZE];
    char commandText[MAX_CONDITIONAL_TEXT_SIZE];
    char commandCondition[MAX_CONDITIONAL_EXPR_SIZE];
} BRIDGEBP;

typedef struct
{
    int count;
    BRIDGEBP* bp;
} BPMAP;

typedef struct
{
    duint start; //OUT
    duint end; //OUT
    duint instrcount; //OUT
} FUNCTION;

typedef struct
{
    int depth; //IN
    duint start; //OUT
    duint end; //OUT
} LOOP;

#ifndef _NO_ADDRINFO
typedef struct
{
    int flags; //ADDRINFOFLAGS (IN)
    char module[MAX_MODULE_SIZE]; //module the address is in
    char label[MAX_LABEL_SIZE];
    char comment[MAX_COMMENT_SIZE];
    bool isbookmark;
    FUNCTION function;
    LOOP loop;
} ADDRINFO;
#endif

struct SYMBOLINFO_
{
    duint addr;
    char* decoratedSymbol;
    char* undecoratedSymbol;
    bool isImported;
};

typedef struct
{
    duint base;
    char name[MAX_MODULE_SIZE];
} SYMBOLMODULEINFO;

typedef struct
{
    duint base;
    CBSYMBOLENUM cbSymbolEnum;
    void* user;
} SYMBOLCBINFO;

typedef struct
{
    bool c;
    bool p;
    bool a;
    bool z;
    bool s;
    bool t;
    bool i;
    bool d;
    bool o;
} FLAGS;

typedef struct
{
    bool FZ;
    bool PM;
    bool UM;
    bool OM;
    bool ZM;
    bool IM;
    bool DM;
    bool DAZ;
    bool PE;
    bool UE;
    bool OE;
    bool ZE;
    bool DE;
    bool IE;

    unsigned short RC;
} MXCSRFIELDS;

typedef struct
{
    bool B;
    bool C3;
    bool C2;
    bool C1;
    bool C0;
    bool IR;
    bool SF;
    bool P;
    bool U;
    bool O;
    bool Z;
    bool D;
    bool I;

    unsigned short TOP;

} X87STATUSWORDFIELDS;

typedef struct
{
    bool IC;
    bool IEM;
    bool PM;
    bool UM;
    bool OM;
    bool ZM;
    bool DM;
    bool IM;

    unsigned short RC;
    unsigned short PC;

} X87CONTROLWORDFIELDS;

typedef struct _XMMREGISTER
{
    ULONGLONG Low;
    LONGLONG High;
} XMMREGISTER;

typedef struct
{
    XMMREGISTER Low; //XMM/SSE part
    XMMREGISTER High; //AVX part
} YMMREGISTER;

typedef struct
{
    BYTE    data[10];
    int     st_value;
    int     tag;
} X87FPUREGISTER;

typedef struct
{
    WORD   ControlWord;
    WORD   StatusWord;
    WORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    DWORD   Cr0NpxState;
} X87FPU;

typedef struct
{
    ULONG_PTR cax;
    ULONG_PTR ccx;
    ULONG_PTR cdx;
    ULONG_PTR cbx;
    ULONG_PTR csp;
    ULONG_PTR cbp;
    ULONG_PTR csi;
    ULONG_PTR cdi;
#ifdef _WIN64
    ULONG_PTR r8;
    ULONG_PTR r9;
    ULONG_PTR r10;
    ULONG_PTR r11;
    ULONG_PTR r12;
    ULONG_PTR r13;
    ULONG_PTR r14;
    ULONG_PTR r15;
#endif //_WIN64
    ULONG_PTR cip;
    ULONG_PTR eflags;
    unsigned short gs;
    unsigned short fs;
    unsigned short es;
    unsigned short ds;
    unsigned short cs;
    unsigned short ss;
    ULONG_PTR dr0;
    ULONG_PTR dr1;
    ULONG_PTR dr2;
    ULONG_PTR dr3;
    ULONG_PTR dr6;
    ULONG_PTR dr7;
    BYTE RegisterArea[80];
    X87FPU x87fpu;
    DWORD MxCsr;
#ifdef _WIN64
    XMMREGISTER XmmRegisters[16];
    YMMREGISTER YmmRegisters[16];
#else // x86
    XMMREGISTER XmmRegisters[8];
    YMMREGISTER YmmRegisters[8];
#endif
} REGISTERCONTEXT;

typedef struct
{
    DWORD code;
    const char* name;
} LASTERROR;

typedef struct
{
    REGISTERCONTEXT regcontext;
    FLAGS flags;
    X87FPUREGISTER x87FPURegisters[8];
    unsigned long long mmx[8];
    MXCSRFIELDS MxCsrFields;
    X87STATUSWORDFIELDS x87StatusWordFields;
    X87CONTROLWORDFIELDS x87ControlWordFields;
    LASTERROR lastError;
} REGDUMP;

typedef struct
{
    DISASM_ARGTYPE type; //normal/memory
    SEGMENTREG segment;
    char mnemonic[64];
    duint constant; //constant in the instruction (imm/disp)
    duint value; //equal to constant or equal to the register value
    duint memvalue; //memsize:[value]
} DISASM_ARG;

typedef struct
{
    char instruction[64];
    DISASM_INSTRTYPE type;
    int argcount;
    int instr_size;
    DISASM_ARG arg[3];
} DISASM_INSTR;

typedef struct
{
    char color[8]; //hex color-code
    char comment[MAX_COMMENT_SIZE];
} STACK_COMMENT;

typedef struct
{
    int ThreadNumber;
    HANDLE Handle;
    DWORD ThreadId;
    duint ThreadStartAddress;
    duint ThreadLocalBase;
    char threadName[MAX_THREAD_NAME_SIZE];
} THREADINFO;

typedef struct
{
    THREADINFO BasicInfo;
    duint ThreadCip;
    DWORD SuspendCount;
    THREADPRIORITY Priority;
    THREADWAITREASON WaitReason;
    DWORD LastError;
} THREADALLINFO;

typedef struct
{
    int count;
    THREADALLINFO* list;
    int CurrentThread;
} THREADLIST;

typedef struct
{
    duint value; //displacement / addrvalue (rip-relative)
    MEMORY_SIZE size; //byte/word/dword/qword
    char mnemonic[MAX_MNEMONIC_SIZE];
} MEMORY_INFO;

typedef struct
{
    duint value;
    VALUE_SIZE size;
} VALUE_INFO;

//definitions for BASIC_INSTRUCTION_INFO.type
#define TYPE_VALUE 1
#define TYPE_MEMORY 2
#define TYPE_ADDR 4

typedef struct
{
    DWORD type; //value|memory|addr
    VALUE_INFO value; //immediat
    MEMORY_INFO memory;
    duint addr; //addrvalue (jumps + calls)
    bool branch; //jumps/calls
    bool call; //instruction is a call
    int size;
    char instruction[MAX_MNEMONIC_SIZE * 4];
} BASIC_INSTRUCTION_INFO;

typedef struct
{
    SCRIPTBRANCHTYPE type;
    int dest;
    char branchlabel[256];
} SCRIPTBRANCH;

typedef struct
{
    duint addr;
    duint start;
    duint end;
    bool manual;
    int depth;
} FUNCTION_LOOP_INFO;

//Debugger functions
extern const char* DbgInit();
extern void DbgExit();

%pybuffer_mutable_string(unsigned char *dest)
extern bool DbgMemRead(duint va, unsigned char* dest, duint size);
%typemap(in) unsigned char* dest;

%pybuffer_mutable_string(const unsigned char* src)
extern bool DbgMemWrite(duint va, const unsigned char* src, duint size);
%typemap(in) const unsigned char* src;

extern duint DbgMemGetPageSize(duint base);
extern duint DbgMemFindBaseAddr(duint addr, duint* size);
extern bool DbgCmdExec(const char* cmd);
extern bool DbgCmdExecDirect(const char* cmd);
extern bool DbgMemMap(MEMMAP* memmap);
extern bool DbgIsValidExpression(const char* expression);
extern bool DbgIsDebugging();
extern bool DbgIsJumpGoingToExecute(duint addr);

%pybuffer_string(char* text)
extern bool DbgGetLabelAt(duint addr, SEGMENTREG segment, char* text);
extern bool DbgSetLabelAt(duint addr, const char* text);
extern void DbgClearLabelRange(duint start, duint end);

%pybuffer_string(char* text);
extern bool DbgGetCommentAt(duint addr, char* text);

extern bool DbgSetCommentAt(duint addr, const char* text);
extern void DbgClearCommentRange(duint start, duint end);
extern bool DbgGetBookmarkAt(duint addr);
extern bool DbgSetBookmarkAt(duint addr, bool isbookmark);
extern void DbgClearBookmarkRange(duint start, duint end);

%pybuffer_string(char* text);
extern bool DbgGetModuleAt(duint addr, char* text);

extern BPXTYPE DbgGetBpxTypeAt(duint addr);
extern duint DbgValFromString(const char* string);
extern bool DbgGetRegDump(REGDUMP* regdump);
extern bool DbgValToString(const char* string, duint value);
extern bool DbgMemIsValidReadPtr(duint addr);
extern int DbgGetBpList(BPXTYPE type, BPMAP* list);
extern FUNCTYPE DbgGetFunctionTypeAt(duint addr);
extern LOOPTYPE DbgGetLoopTypeAt(duint addr, int depth);
extern duint DbgGetBranchDestination(duint addr);
extern void DbgScriptLoad(const char* filename);
extern void DbgScriptUnload();
extern void DbgScriptRun(int destline);
extern void DbgScriptStep();
extern bool DbgScriptBpToggle(int line);
extern bool DbgScriptBpGet(int line);
extern bool DbgScriptCmdExec(const char* command);
extern void DbgScriptAbort();
extern SCRIPTLINETYPE DbgScriptGetLineType(int line);
extern void DbgScriptSetIp(int line);
extern bool DbgScriptGetBranchInfo(int line, SCRIPTBRANCH* info);
extern void DbgSymbolEnum(duint base, CBSYMBOLENUM cbSymbolEnum, void* user);
extern void DbgSymbolEnumFromCache(duint base, CBSYMBOLENUM cbSymbolEnum, void* user);
extern bool DbgAssembleAt(duint addr, const char* instruction);
extern duint DbgModBaseFromName(const char* name);
extern void DbgDisasmAt(duint addr, DISASM_INSTR* instr);
extern bool DbgStackCommentGet(duint addr, STACK_COMMENT* comment);
extern void DbgGetThreadList(THREADLIST* list);
extern void DbgSettingsUpdated();
extern void DbgDisasmFastAt(duint addr, BASIC_INSTRUCTION_INFO* basicinfo);
extern void DbgMenuEntryClicked(int hEntry);
extern bool DbgFunctionGet(duint addr, duint* start, duint* end);
extern bool DbgFunctionOverlaps(duint start, duint end);
extern bool DbgFunctionAdd(duint start, duint end);
extern bool DbgFunctionDel(duint addr);
extern bool DbgLoopGet(int depth, duint addr, duint* start, duint* end);
extern bool DbgLoopOverlaps(int depth, duint start, duint end);
extern bool DbgLoopAdd(duint start, duint end);
extern bool DbgLoopDel(int depth, duint addr);
extern bool DbgIsRunLocked();
extern bool DbgIsBpDisabled(duint addr);
extern bool DbgSetAutoCommentAt(duint addr, const char* text);
extern void DbgClearAutoCommentRange(duint start, duint end);
extern bool DbgSetAutoLabelAt(duint addr, const char* text);
extern void DbgClearAutoLabelRange(duint start, duint end);
extern bool DbgSetAutoBookmarkAt(duint addr);
extern void DbgClearAutoBookmarkRange(duint start, duint end);
extern bool DbgSetAutoFunctionAt(duint start, duint end);
extern void DbgClearAutoFunctionRange(duint start, duint end);

%pybuffer_string(char* text);
extern bool DbgGetStringAt(duint addr, char* text);

extern DBGFUNCTIONS* DbgFunctions();
extern bool DbgWinEvent(MSG* message, long* result);
extern bool DbgWinEventGlobal(MSG* message);
extern bool DbgIsRunning();
extern duint DbgGetTimeWastedCounter();
extern ARGTYPE DbgGetArgTypeAt(duint addr);

//Gui defines
#define GUI_PLUGIN_MENU 0
#define GUI_DISASM_MENU 1
#define GUI_DUMP_MENU 2
#define GUI_STACK_MENU 3

#define GUI_DISASSEMBLY 0
#define GUI_DUMP 1
#define GUI_STACK 2

#define GUI_MAX_LINE_SIZE 65536
#define GUI_MAX_DISASSEMBLY_SIZE 2048

//Gui enums
typedef enum
{
    GUI_DISASSEMBLE_AT,             // param1=(duint)va,            param2=(duint)cip
    GUI_SET_DEBUG_STATE,            // param1=(DBGSTATE)state,      param2=unused
    GUI_ADD_MSG_TO_LOG,             // param1=(const char*)msg,     param2=unused
    GUI_CLEAR_LOG,                  // param1=unused,               param2=unused
    GUI_UPDATE_REGISTER_VIEW,       // param1=unused,               param2=unused
    GUI_UPDATE_DISASSEMBLY_VIEW,    // param1=unused,               param2=unused
    GUI_UPDATE_BREAKPOINTS_VIEW,    // param1=unused,               param2=unused
    GUI_UPDATE_WINDOW_TITLE,        // param1=(const char*)file,    param2=unused
    GUI_GET_WINDOW_HANDLE,          // param1=unused,               param2=unused
    GUI_DUMP_AT,                    // param1=(duint)va             param2=unused
    GUI_SCRIPT_ADD,                 // param1=int count,            param2=const char** lines
    GUI_SCRIPT_CLEAR,               // param1=unused,               param2=unused
    GUI_SCRIPT_SETIP,               // param1=int line,             param2=unused
    GUI_SCRIPT_ERROR,               // param1=int line,             param2=const char* message
    GUI_SCRIPT_SETTITLE,            // param1=const char* title,    param2=unused
    GUI_SCRIPT_SETINFOLINE,         // param1=int line,             param2=const char* info
    GUI_SCRIPT_MESSAGE,             // param1=const char* message,  param2=unused
    GUI_SCRIPT_MSGYN,               // param1=const char* message,  param2=unused
    GUI_SYMBOL_LOG_ADD,             // param1(const char*)msg,      param2=unused
    GUI_SYMBOL_LOG_CLEAR,           // param1=unused,               param2=unused
    GUI_SYMBOL_SET_PROGRESS,        // param1=int percent           param2=unused
    GUI_SYMBOL_UPDATE_MODULE_LIST,  // param1=int count,            param2=SYMBOLMODULEINFO* modules
    GUI_REF_ADDCOLUMN,              // param1=int width,            param2=(const char*)title
    GUI_REF_SETROWCOUNT,            // param1=int rows,             param2=unused
    GUI_REF_GETROWCOUNT,            // param1=unused,               param2=unused
    GUI_REF_DELETEALLCOLUMNS,       // param1=unused,               param2=unused
    GUI_REF_SETCELLCONTENT,         // param1=(CELLINFO*)info,      param2=unused
    GUI_REF_GETCELLCONTENT,         // param1=int row,              param2=int col
    GUI_REF_RELOADDATA,             // param1=unused,               param2=unused
    GUI_REF_SETSINGLESELECTION,     // param1=int index,            param2=bool scroll
    GUI_REF_SETPROGRESS,            // param1=int progress,         param2=unused
    GUI_REF_SETCURRENTTASKPROGRESS, // param1=int progress,         param2=const char* taskTitle
    GUI_REF_SETSEARCHSTARTCOL,      // param1=int col               param2=unused
    GUI_STACK_DUMP_AT,              // param1=duint addr,           param2=duint csp
    GUI_UPDATE_DUMP_VIEW,           // param1=unused,               param2=unused
    GUI_UPDATE_THREAD_VIEW,         // param1=unused,               param2=unused
    GUI_ADD_RECENT_FILE,            // param1=(const char*)file,    param2=unused
    GUI_SET_LAST_EXCEPTION,         // param1=unsigned int code,    param2=unused
    GUI_GET_DISASSEMBLY,            // param1=duint addr,           param2=char* text
    GUI_MENU_ADD,                   // param1=int hMenu,            param2=const char* title
    GUI_MENU_ADD_ENTRY,             // param1=int hMenu,            param2=const char* title
    GUI_MENU_ADD_SEPARATOR,         // param1=int hMenu,            param2=unused
    GUI_MENU_CLEAR,                 // param1=int hMenu,            param2=unused
    GUI_SELECTION_GET,              // param1=int hWindow,          param2=SELECTIONDATA* selection
    GUI_SELECTION_SET,              // param1=int hWindow,          param2=const SELECTIONDATA* selection
    GUI_GETLINE_WINDOW,             // param1=const char* title,    param2=char* text
    GUI_AUTOCOMPLETE_ADDCMD,        // param1=const char* cmd,      param2=ununsed
    GUI_AUTOCOMPLETE_DELCMD,        // param1=const char* cmd,      param2=ununsed
    GUI_AUTOCOMPLETE_CLEARALL,      // param1=unused,               param2=unused
    GUI_SCRIPT_ENABLEHIGHLIGHTING,  // param1=bool enable,          param2=unused
    GUI_ADD_MSG_TO_STATUSBAR,       // param1=const char* msg,      param2=unused
    GUI_UPDATE_SIDEBAR,             // param1=unused,               param2=unused
    GUI_REPAINT_TABLE_VIEW,         // param1=unused,               param2=unused
    GUI_UPDATE_PATCHES,             // param1=unused,               param2=unused
    GUI_UPDATE_CALLSTACK,           // param1=unused,               param2=unused
    GUI_UPDATE_SEHCHAIN,            // param1=unused,               param2=unused
    GUI_SYMBOL_REFRESH_CURRENT,     // param1=unused,               param2=unused
    GUI_UPDATE_MEMORY_VIEW,         // param1=unused,               param2=unused
    GUI_REF_INITIALIZE,             // param1=const char* name,     param2=unused
    GUI_LOAD_SOURCE_FILE,           // param1=const char* path,     param2=line
    GUI_MENU_SET_ICON,              // param1=int hMenu,            param2=ICONINFO*
    GUI_MENU_SET_ENTRY_ICON,        // param1=int hEntry,           param2=ICONINFO*
    GUI_SHOW_CPU,                   // param1=unused,               param2=unused
    GUI_ADD_QWIDGET_TAB,            // param1=QWidget*,             param2=unused
    GUI_SHOW_QWIDGET_TAB,           // param1=QWidget*,             param2=unused
    GUI_CLOSE_QWIDGET_TAB,          // param1=QWidget*,             param2=unused
    GUI_EXECUTE_ON_GUI_THREAD,      // param1=GUICALLBACK,          param2=unused
    GUI_UPDATE_TIME_WASTED_COUNTER, // param1=unused,               param2=unused
    GUI_SET_GLOBAL_NOTES,           // param1=const char* text,     param2=unused
    GUI_GET_GLOBAL_NOTES,           // param1=char** text,          param2=unused
    GUI_SET_DEBUGGEE_NOTES,         // param1=const char* text,     param2=unused
    GUI_GET_DEBUGGEE_NOTES,         // param1=char** text,          param2=unused
    GUI_DUMP_AT_N,                  // param1=int index,            param2=duint va
    GUI_DISPLAY_WARNING,            // param1=const char *text,     param2=unused
    GUI_REGISTER_SCRIPT_LANG,       // param1=SCRIPTTYPEINFO* info, param2=unused
    GUI_UNREGISTER_SCRIPT_LANG,     // param1=int id,               param2=unused
    GUI_UPDATE_ARGUMENT_VIEW,       // param1=unused,               param2=unused
    GUI_FOCUS_VIEW,                 // param1=int hWindow,          param2=unused
} GUIMSG;

//GUI Typedefs
typedef void (*GUICALLBACK)();
typedef bool (*GUISCRIPTEXECUTE)(const char* text);
typedef void (*GUISCRIPTCOMPLETER)(const char* text, char** entries, int* entryCount);

//GUI structures
typedef struct
{
    int row;
    int col;
    const char* str;
} CELLINFO;

typedef struct
{
    duint start;
    duint end;
} SELECTIONDATA;

typedef struct
{
    const void* data;
    duint size;
} ICONDATA;

typedef struct
{
    char name[64];
    int id;
    GUISCRIPTEXECUTE execute;
    GUISCRIPTCOMPLETER completeCommand;
} SCRIPTTYPEINFO;

//GUI functions
extern void GuiDisasmAt(duint addr, duint cip);
extern void GuiSetDebugState(DBGSTATE state);
extern void GuiAddLogMessage(const char* msg);
extern void GuiLogClear();
extern void GuiUpdateAllViews();
extern void GuiUpdateRegisterView();
extern void GuiUpdateDisassemblyView();
extern void GuiUpdateBreakpointsView();
extern void GuiUpdateWindowTitle(const char* filename);
extern HWND GuiGetWindowHandle();
extern void GuiDumpAt(duint va);
extern void GuiScriptAdd(int count, const char** lines);
extern void GuiScriptClear();
extern void GuiScriptSetIp(int line);
extern void GuiScriptError(int line, const char* message);
extern void GuiScriptSetTitle(const char* title);
extern void GuiScriptSetInfoLine(int line, const char* info);
extern void GuiScriptMessage(const char* message);
extern int GuiScriptMsgyn(const char* message);
extern void GuiScriptEnableHighlighting(bool enable);
extern void GuiSymbolLogAdd(const char* message);
extern void GuiSymbolLogClear();
extern void GuiSymbolSetProgress(int percent);
extern void GuiSymbolUpdateModuleList(int count, SYMBOLMODULEINFO* modules);
extern void GuiSymbolRefreshCurrent();
extern void GuiReferenceAddColumn(int width, const char* title);
extern void GuiReferenceSetRowCount(int count);
extern int GuiReferenceGetRowCount();
extern void GuiReferenceDeleteAllColumns();
extern void GuiReferenceInitialize(const char* name);
extern void GuiReferenceSetCellContent(int row, int col, const char* str);
extern const char* GuiReferenceGetCellContent(int row, int col);
extern void GuiReferenceReloadData();
extern void GuiReferenceSetSingleSelection(int index, bool scroll);
extern void GuiReferenceSetProgress(int progress);
extern void GuiReferenceSetCurrentTaskProgress(int progress, const char* taskTitle);
extern void GuiReferenceSetSearchStartCol(int col);
extern void GuiStackDumpAt(duint addr, duint csp);
extern void GuiUpdateDumpView();
extern void GuiUpdateThreadView();
extern void GuiUpdateMemoryView();
extern void GuiAddRecentFile(const char* file);
extern void GuiSetLastException(unsigned int exception);

%pybuffer_string(char* text)
extern bool GuiGetDisassembly(duint addr, char* text);
extern int GuiMenuAdd(int hMenu, const char* title);
extern int GuiMenuAddEntry(int hMenu, const char* title);
extern void GuiMenuAddSeparator(int hMenu);
extern void GuiMenuClear(int hMenu);
extern bool GuiSelectionGet(int hWindow, SELECTIONDATA* selection);
extern bool GuiSelectionSet(int hWindow, const SELECTIONDATA* selection);

%pybuffer_string(char* text);
extern bool GuiGetLineWindow(const char* title, char* text);

extern void GuiAutoCompleteAddCmd(const char* cmd);
extern void GuiAutoCompleteDelCmd(const char* cmd);
extern void GuiAutoCompleteClearAll();
extern void GuiAddStatusBarMessage(const char* msg);
extern void GuiUpdateSideBar();
extern void GuiRepaintTableView();
extern void GuiUpdatePatches();
extern void GuiUpdateCallStack();
extern void GuiUpdateSEHChain();
extern void GuiLoadSourceFile(const char* path, int line);
extern void GuiMenuSetIcon(int hMenu, const ICONDATA* icon);
extern void GuiMenuSetEntryIcon(int hEntry, const ICONDATA* icon);
extern void GuiShowCpu();
extern void GuiAddQWidgetTab(void* qWidget);
extern void GuiShowQWidgetTab(void* qWidget);
extern void GuiCloseQWidgetTab(void* qWidget);
extern void GuiExecuteOnGuiThread(GUICALLBACK cbGuiThread);
extern void GuiUpdateTimeWastedCounter();
extern void GuiSetGlobalNotes(const char* text);
extern void GuiGetGlobalNotes(char** text);
extern void GuiSetDebuggeeNotes(const char* text);
extern void GuiGetDebuggeeNotes(char** text);
extern void GuiDumpAtN(duint va, int index);
extern void GuiDisplayWarning(const char* title, const char* text);
extern void GuiRegisterScriptLanguage(SCRIPTTYPEINFO* info);
extern void GuiUnregisterScriptLanguage(int id);
extern void GuiUpdateArgumentWidget();
extern void GuiFocusView(int hWindow);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif // _BRIDGEMAIN_H_
