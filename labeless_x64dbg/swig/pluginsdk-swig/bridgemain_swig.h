#ifndef _BRIDGEMAIN_H_
#define _BRIDGEMAIN_H_

#include <windows.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

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

#ifndef BRIDGE_IMPEXP
#ifdef BUILD_BRIDGE
#define BRIDGE_IMPEXP __declspec(dllexport)
#else
#define BRIDGE_IMPEXP __declspec(dllimport)
#endif //BUILD_BRIDGE
#endif //BRIDGE_IMPEXP

#ifdef __cplusplus
extern "C"
{
#endif

//Bridge defines
#define MAX_SETTING_SIZE 65536
#define DBG_VERSION 25

//Bridge functions

/// <summary>
/// Initialize the bridge.
/// </summary>
/// <returns>On error it returns a non-null error message.</returns>
extern const wchar_t* BridgeInit();

/// <summary>
/// Start the bridge.
/// </summary>
/// <returns>On error it returns a non-null error message.</returns>
extern const wchar_t* BridgeStart();

/// <summary>
/// Allocate buffer. Use BridgeFree to free the buffer.
/// </summary>
/// <param name="size">Size in bytes of the buffer to allocate.</param>
/// <returns>A pointer to the allocated buffer. This function will trigger a crash dump if unsuccessful.</returns>
extern void* BridgeAlloc(size_t size);

/// <summary>
/// Free buffer allocated by BridgeAlloc.
/// </summary>
/// <param name="ptr">Buffer to free.</param>
extern void BridgeFree(void* ptr);

/// <summary>
/// Get a string setting from the in-memory setting store.
/// </summary>
/// <param name="section">Section the setting is in. Cannot be null.</param>
/// <param name="key">Setting key (name). Cannot be null.</param>
/// <param name="value">Output buffer for the value. Should be of MAX_SETTING_SIZE. Cannot be null.</param>
/// <returns>True if the setting was found and copied in the value parameter.</returns>
extern bool BridgeSettingGet(const char* section, const char* key, char* value);

/// <summary>
/// Get an integer setting from the in-memory setting store.
/// </summary>
/// <param name="section">Section the setting is in. Cannot be null.</param>
/// <param name="key">Setting key (name). Cannot be null.</param>
/// <param name="value">Output value.</param>
/// <returns>True if the setting was found and successfully converted to an integer.</returns>
extern bool BridgeSettingGetUint(const char* section, const char* key, duint* value);

/// <summary>
/// Set a string setting in the in-memory setting store.
/// </summary>
/// <param name="section">Section the setting is in. Cannot be null.</param>
/// <param name="key">Setting key (name). Set to null to clear the whole section.</param>
/// <param name="value">New setting value. Set to null to remove the key from the section.</param>
/// <returns>True if the operation was successful.</returns>
extern bool BridgeSettingSet(const char* section, const char* key, const char* value);

/// <summary>
/// Set an integer setting in the in-memory setting store.
/// </summary>
/// <param name="section">Section the setting is in. Cannot be null.</param>
/// <param name="key">Setting key (name). Set to null to clear the whole section.</param>
/// <param name="value">New setting value.</param>
/// <returns>True if the operation was successful.</returns>
extern bool BridgeSettingSetUint(const char* section, const char* key, duint value);

/// <summary>
/// Flush the in-memory setting store to disk.
/// </summary>
/// <returns></returns>
extern bool BridgeSettingFlush();

/// <summary>
/// Read the in-memory setting store from disk.
/// </summary>
/// <param name="errorLine">Line where the error occurred. Set to null to ignore this.</param>
/// <returns>True if the setting were read and parsed correctly.</returns>
extern bool BridgeSettingRead(int* errorLine);

/// <summary>
/// Get the debugger version.
/// </summary>
/// <returns>25</returns>
extern int BridgeGetDbgVersion();

#ifdef __cplusplus
}
#endif

//list structure (and C++ wrapper)
#include "bridgelist.h"

#include "bridgegraph.h"

#ifdef __cplusplus
extern "C"
{
#endif

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
#define MAX_WATCH_NAME_SIZE 256
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
    flagmodule = 0x1,
    flaglabel = 0x2,
    flagcomment = 0x4,
    flagbookmark = 0x8,
    flagfunction = 0x10,
    flagloop = 0x20,
    flagargs = 0x40,
    flagNoFuncOffset = 0x80
} ADDRINFOFLAGS;

typedef enum
{
    bp_none = 0,
    bp_normal = 1,
    bp_hardware = 2,
    bp_memory = 4,
    bp_dll = 8,
    bp_exception = 16
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
    LOOP_END,
    LOOP_SINGLE
} LOOPTYPE;

//order by most important type last
typedef enum
{
    XREF_NONE,
    XREF_DATA,
    XREF_JMP,
    XREF_CALL
} XREFTYPE;

typedef enum
{
    ARG_NONE,
    ARG_BEGIN,
    ARG_MIDDLE,
    ARG_END,
    ARG_SINGLE
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
    DBG_GET_XREF_COUNT_AT,          // param1=duint addr,                param2=unused
    DBG_GET_XREF_TYPE_AT,           // param1=duint addr,                param2=unused
    DBG_XREF_ADD,                   // param1=duint addr,                param2=duint from
    DBG_XREF_DEL_ALL,               // param1=duint addr,                param2=unused
    DBG_XREF_GET,                   // param1=duint addr,                param2=XREF_INFO* info
    DBG_GET_ENCODE_TYPE_BUFFER,     // param1=duint addr,                param2=unused
    DBG_ENCODE_TYPE_GET,            // param1=duint addr,                param2=duint size
    DBG_DELETE_ENCODE_TYPE_RANGE,   // param1=duint start,               param2=duint end
    DBG_ENCODE_SIZE_GET,            // param1=duint addr,                param2=duint codesize
    DBG_DELETE_ENCODE_TYPE_SEG,     // param1=duint addr,                param2=unused
    DBG_RELEASE_ENCODE_TYPE_BUFFER, // param1=void* buffer,              param2=unused
    DBG_ARGUMENT_GET,               // param1=FUNCTION* info,            param2=unused
    DBG_ARGUMENT_OVERLAPS,          // param1=FUNCTION* info,            param2=unused
    DBG_ARGUMENT_ADD,               // param1=FUNCTION* info,            param2=unused
    DBG_ARGUMENT_DEL,               // param1=FUNCTION* info,            param2=unused
    DBG_GET_WATCH_LIST,             // param1=ListOf(WATCHINFO),         param2=unused
    DBG_SELCHANGED,                 // param1=hWindow,                   param2=VA
    DBG_GET_PROCESS_HANDLE,         // param1=unused,                    param2=unused
    DBG_GET_THREAD_HANDLE,          // param1=unused,                    param2=unused
    DBG_GET_PROCESS_ID,             // param1=unused,                    param2=unused
    DBG_GET_THREAD_ID,              // param1=unused,                    param2=unused
    DBG_GET_PEB_ADDRESS,            // param1=DWORD ProcessId,           param2=unused
    DBG_GET_TEB_ADDRESS,            // param1=DWORD ThreadId,            param2=unused
    DBG_ANALYZE_FUNCTION,           // param1=BridgeCFGraphList* graph,  param2=duint entry
    DBG_MENU_PREPARE,               // param1=int hMenu,                 param2=unused
    DBG_GET_SYMBOL_INFO,            // param1=void* symbol,              param2=SYMBOLINFO* info
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

typedef enum
{
    enc_unknown,  //must be 0
    enc_byte,     //1 byte
    enc_word,     //2 bytes
    enc_dword,    //4 bytes
    enc_fword,    //6 bytes
    enc_qword,    //8 bytes
    enc_tbyte,    //10 bytes
    enc_oword,    //16 bytes
    enc_mmword,   //8 bytes
    enc_xmmword,  //16 bytes
    enc_ymmword,  //32 bytes
    enc_zmmword,  //64 bytes avx512 not supported
    enc_real4,    //4 byte float
    enc_real8,    //8 byte double
    enc_real10,   //10 byte decimal
    enc_ascii,    //ascii sequence
    enc_unicode,  //unicode sequence
    enc_code,     //start of code
    enc_junk,     //junk code
    enc_middle    //middle of data
} ENCODETYPE;

typedef enum
{
    TYPE_UINT, // unsigned integer
    TYPE_INT,  // signed integer
    TYPE_FLOAT,// single precision floating point value
    TYPE_ASCII, // ascii string
    TYPE_UNICODE, // unicode string
    TYPE_INVALID // invalid watch expression or data type
} WATCHVARTYPE;

typedef enum
{
    MODE_DISABLED, // watchdog is disabled
    MODE_ISTRUE,   // alert if expression is not 0
    MODE_ISFALSE,  // alert if expression is 0
    MODE_CHANGED,  // alert if expression is changed
    MODE_UNCHANGED // alert if expression is not changed
} WATCHDOGMODE;

typedef enum
{
    hw_access,
    hw_write,
    hw_execute
} BPHWTYPE;

typedef enum
{
    mem_access,
    mem_read,
    mem_write,
    mem_execute
} BPMEMTYPE;

typedef enum
{
    dll_load = 1,
    dll_unload,
    dll_all
} BPDLLTYPE;

typedef enum
{
    ex_firstchance = 1,
    ex_secondchance,
    ex_all
} BPEXTYPE;

typedef enum
{
    hw_byte,
    hw_word,
    hw_dword,
    hw_qword
} BPHWSIZE;

typedef enum
{
    sym_import,
    sym_export,
    sym_symbol
} SYMBOLTYPE;

//Debugger typedefs
typedef MEMORY_SIZE VALUE_SIZE;
//typedef struct SYMBOLINFO_ SYMBOLINFO;
struct SYMBOLPTR_; // fwd
#include "_dbgfunctions.h" // typedef struct DBGFUNCTIONS_ DBGFUNCTIONS;

typedef bool (*CBSYMBOLENUM)(const struct SYMBOLPTR_* symbol, void* user);

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
    unsigned char typeEx; //BPHWTYPE/BPMEMTYPE/BPDLLTYPE/BPEXTYPE
    unsigned char hwSize; //BPHWSIZE
    unsigned int hitCount;
    bool fastResume;
    bool silent;
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
    char WatchName[MAX_WATCH_NAME_SIZE];
    char Expression[MAX_CONDITIONAL_EXPR_SIZE];
    unsigned int window;
    unsigned int id;
    WATCHVARTYPE varType;
    WATCHDOGMODE watchdogMode;
    duint value;
    bool watchdogTriggered;
} WATCHINFO;

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
    duint instrcount; //OUT
} LOOP;

typedef struct
{
    int flags; //ADDRINFOFLAGS (IN)
    char module[MAX_MODULE_SIZE]; //module the address is in
    char label[MAX_LABEL_SIZE];
    char comment[MAX_COMMENT_SIZE];
    bool isbookmark;
    FUNCTION function;
    LOOP loop;
    FUNCTION args;
} BRIDGE_ADDRINFO;

typedef struct SYMBOLINFO_
{
    duint addr;
    char* decoratedSymbol;
    char* undecoratedSymbol;
    SYMBOLTYPE type;
    bool freeDecorated;
    bool freeUndecorated;
} SYMBOLINFO;

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
    bool ES;
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

typedef struct DECLSPEC_ALIGN(16) _XMMREGISTER
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
    char name[128];
} LASTERROR;

typedef struct
{
    DWORD code;
    char name[128];
} LASTSTATUS;

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
    LASTSTATUS lastStatus;
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
    FILETIME UserTime;
    FILETIME KernelTime;
    FILETIME CreationTime;
    ULONG64 Cycles; // Windows Vista or greater
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

typedef struct
{
    duint addr;
    XREFTYPE type;
} XREF_RECORD;

typedef struct
{
    duint refcount;
    XREF_RECORD* references;
} XREF_INFO;

typedef struct SYMBOLPTR_
{
    duint modbase;
    const void* symbol;
} SYMBOLPTR;

//Debugger functions
extern const char* DbgInit();
extern void DbgExit();
%pybuffer_mutable_string(void *dest)
extern bool DbgMemRead(duint va, void* dest, duint size);
%typemap(in) void* dest;
%pybuffer_mutable_string(const unsigned char* src)
extern bool DbgMemWrite(duint va, const void* src, duint size);
%typemap(in) const void* src;
extern duint DbgMemGetPageSize(duint base);
extern duint DbgMemFindBaseAddr(duint addr, duint* size);

/// <summary>
/// Asynchronously execute a debugger command by adding it to the command queue.
/// Note: the command may not have completed before this call returns. Use this
/// function if you don't care when the command gets executed.
///
/// Example: DbgCmdExec("ClearLog")
/// </summary>
/// <param name="cmd">The command to execute.</param>
/// <returns>True if the command was successfully submitted to the command queue. False if the submission failed.</returns>
extern bool DbgCmdExec(const char* cmd);

/// <summary>
/// Performs synchronous execution of a debugger command. This function call only
/// returns after the command has completed.
///
/// Example: DbgCmdExecDirect("loadlib advapi32.dll")
/// </summary>
/// <param name="cmd">The command to execute.</param>
/// <returns>True if the command executed successfully, False if there was a problem.</returns>
extern bool DbgCmdExecDirect(const char* cmd);
extern bool DbgMemMap(MEMMAP* memmap);
extern bool DbgIsValidExpression(const char* expression);
extern bool DbgIsDebugging();
extern bool DbgIsJumpGoingToExecute(duint addr);
%pybuffer_string(char* text)
extern bool DbgGetLabelAt(duint addr, SEGMENTREG segment, char* text);
%pythoncode %{
def DbgGetLabelAt(addr, segment):
    import ctypes as C
    buff = C.create_string_buffer(_x64dbgapi.MAX_LABEL_SIZE)
    if not _x64dbgapi.DbgGetLabelAt(addr, segment, buff):
        return
    return buff.value.replace('\0', '')
%}
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
extern bool DbgGetRegDumpEx(REGDUMP* regdump, size_t size);
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
extern bool DbgArgumentGet(duint addr, duint* start, duint* end);
extern bool DbgArgumentOverlaps(duint start, duint end);
extern bool DbgArgumentAdd(duint start, duint end);
extern bool DbgArgumentDel(duint addr);
extern bool DbgLoopGet(int depth, duint addr, duint* start, duint* end);
extern bool DbgLoopOverlaps(int depth, duint start, duint end);
extern bool DbgLoopAdd(duint start, duint end);
extern bool DbgLoopDel(int depth, duint addr);
extern bool DbgXrefAdd(duint addr, duint from);
extern bool DbgXrefDelAll(duint addr);
extern bool DbgXrefGet(duint addr, XREF_INFO* info);
extern size_t DbgGetXrefCountAt(duint addr);
extern XREFTYPE DbgGetXrefTypeAt(duint addr);
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
extern const DBGFUNCTIONS* DbgFunctions();
extern bool DbgWinEvent(MSG* message, long* result);
extern bool DbgWinEventGlobal(MSG* message);
extern bool DbgIsRunning();
extern duint DbgGetTimeWastedCounter();
extern ARGTYPE DbgGetArgTypeAt(duint addr);
extern void* DbgGetEncodeTypeBuffer(duint addr, duint* size);
extern void DbgReleaseEncodeTypeBuffer(void* buffer);
extern ENCODETYPE DbgGetEncodeTypeAt(duint addr, duint size);
extern duint DbgGetEncodeSizeAt(duint addr, duint codesize);
extern bool DbgSetEncodeType(duint addr, duint size, ENCODETYPE type);
extern void DbgDelEncodeTypeRange(duint start, duint end);
extern void DbgDelEncodeTypeSegment(duint start);
extern bool DbgGetWatchList(ListOf(WATCHINFO) list);
extern void DbgSelChanged(int hWindow, duint VA);
extern HANDLE DbgGetProcessHandle();
extern HANDLE DbgGetThreadHandle();
extern DWORD DbgGetProcessId();
extern DWORD DbgGetThreadId();
extern duint DbgGetPebAddress(DWORD ProcessId);
extern duint DbgGetTebAddress(DWORD ThreadId);
extern bool DbgAnalyzeFunction(duint entry, BridgeCFGraphList* graph);
extern duint DbgEval(const char* expression, bool* success = 0);
extern void DbgMenuPrepare(int hMenu);
extern void DbgGetSymbolInfo(const SYMBOLPTR* symbolptr, SYMBOLINFO* info);

//Gui defines
#define GUI_PLUGIN_MENU 0
#define GUI_DISASM_MENU 1
#define GUI_DUMP_MENU 2
#define GUI_STACK_MENU 3

#define GUI_DISASSEMBLY 0
#define GUI_DUMP 1
#define GUI_STACK 2
#define GUI_GRAPH 3
#define GUI_MEMMAP 4
#define GUI_SYMMOD 5

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
    GUI_LOAD_SOURCE_FILE,           // param1=const char* path,     param2=duint addr
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
    GUI_UPDATE_WATCH_VIEW,          // param1=unused,               param2=unused
    GUI_LOAD_GRAPH,                 // param1=BridgeCFGraphList*    param2=unused
    GUI_GRAPH_AT,                   // param1=duint addr            param2=unused
    GUI_UPDATE_GRAPH_VIEW,          // param1=unused,               param2=unused
    GUI_SET_LOG_ENABLED,            // param1=bool isEnabled        param2=unused
    GUI_ADD_FAVOURITE_TOOL,         // param1=const char* name      param2=const char* description
    GUI_ADD_FAVOURITE_COMMAND,      // param1=const char* command   param2=const char* shortcut
    GUI_SET_FAVOURITE_TOOL_SHORTCUT,// param1=const char* name      param2=const char* shortcut
    GUI_FOLD_DISASSEMBLY,           // param1=duint startAddress    param2=duint length
    GUI_SELECT_IN_MEMORY_MAP,       // param1=duint addr,           param2=unused
    GUI_GET_ACTIVE_VIEW,            // param1=ACTIVEVIEW*,          param2=unused
    GUI_MENU_SET_ENTRY_CHECKED,     // param1=int hEntry,           param2=bool checked
    GUI_ADD_INFO_LINE,              // param1=const char* infoline, param2=unused
    GUI_PROCESS_EVENTS,             // param1=unused,               param2=unused
    GUI_TYPE_ADDNODE,               // param1=void* parent,         param2=TYPEDESCRIPTOR* type
    GUI_TYPE_CLEAR,                 // param1=unused,               param2=unused
    GUI_UPDATE_TYPE_WIDGET,         // param1=unused,               param2=unused
    GUI_CLOSE_APPLICATION,          // param1=unused,               param2=unused
    GUI_MENU_SET_VISIBLE,           // param1=int hMenu,            param2=bool visible
    GUI_MENU_SET_ENTRY_VISIBLE,     // param1=int hEntry,           param2=bool visible
    GUI_MENU_SET_NAME,              // param1=int hMenu,            param2=const char* name
    GUI_MENU_SET_ENTRY_NAME,        // param1=int hEntry,           param2=const char* name
    GUI_FLUSH_LOG,                  // param1=unused,               param2=unused
    GUI_MENU_SET_ENTRY_HOTKEY,      // param1=int hEntry,           param2=const char* hack
    GUI_REF_SEARCH_GETROWCOUNT,     // param1=unused,               param2=unused
    GUI_REF_SEARCH_GETCELLCONTENT,  // param1=int row,              param2=int col
    GUI_MENU_REMOVE,                // param1=int hEntryMenu,       param2=unused
    GUI_REF_ADDCOMMAND,             // param1=const char* title,    param2=const char* command
    GUI_OPEN_TRACE_FILE,            // param1=const char* file name,param2=unused
    GUI_UPDATE_TRACE_BROWSER,       // param1=unused,               param2=unused
    GUI_INVALIDATE_SYMBOL_SOURCE,   // param1=duint base,           param2=unused
} GUIMSG;

//GUI Typedefs
struct _TYPEDESCRIPTOR;

typedef void (*GUICALLBACK)();
typedef bool (*GUISCRIPTEXECUTE)(const char* text);
typedef void (*GUISCRIPTCOMPLETER)(const char* text, char** entries, int* entryCount);
typedef bool (*TYPETOSTRING)(const struct _TYPEDESCRIPTOR* type, char* dest, size_t* destCount); //don't change destCount for final failure

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

typedef struct
{
    void* titleHwnd;
    void* classHwnd;
    char title[MAX_STRING_SIZE];
    char className[MAX_STRING_SIZE];
} ACTIVEVIEW;

typedef struct _TYPEDESCRIPTOR
{
    bool expanded; //is the type node expanded?
    bool reverse; //big endian?
    const char* name; //type name (int b)
    duint addr; //virtual address
    duint offset; //offset to addr for the actual location
    int id; //type id
    int size; //sizeof(type)
    TYPETOSTRING callback; //convert to string
    void* userdata; //user data
} TYPEDESCRIPTOR;

//GUI functions
//code page is utf8
extern const char* GuiTranslateText(const char* Source);
extern void GuiDisasmAt(duint addr, duint cip);
extern void GuiSetDebugState(DBGSTATE state);
extern void GuiSetDebugStateFast(DBGSTATE state);
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
extern int GuiReferenceSearchGetRowCount();
extern void GuiReferenceDeleteAllColumns();
extern void GuiReferenceInitialize(const char* name);
extern void GuiReferenceSetCellContent(int row, int col, const char* str);
extern const char* GuiReferenceGetCellContent(int row, int col);
extern const char* GuiReferenceSearchGetCellContent(int row, int col);
extern void GuiReferenceReloadData();
extern void GuiReferenceSetSingleSelection(int index, bool scroll);
extern void GuiReferenceSetProgress(int progress);
extern void GuiReferenceSetCurrentTaskProgress(int progress, const char* taskTitle);
extern void GuiReferenceSetSearchStartCol(int col);
extern void GuiStackDumpAt(duint addr, duint csp);
extern void GuiUpdateDumpView();
extern void GuiUpdateWatchView();
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
extern void GuiMenuRemove(int hEntryMenu);
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
extern void GuiLoadSourceFileEx(const char* path, duint addr);
extern void GuiMenuSetIcon(int hMenu, const ICONDATA* icon);
extern void GuiMenuSetEntryIcon(int hEntry, const ICONDATA* icon);
extern void GuiMenuSetEntryChecked(int hEntry, bool checked);
extern void GuiMenuSetVisible(int hMenu, bool visible);
extern void GuiMenuSetEntryVisible(int hEntry, bool visible);
extern void GuiMenuSetName(int hMenu, const char* name);
extern void GuiMenuSetEntryName(int hEntry, const char* name);
extern void GuiMenuSetEntryHotkey(int hEntry, const char* hack);
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
extern bool GuiIsUpdateDisabled();
extern void GuiUpdateEnable(bool updateNow);
extern void GuiUpdateDisable();
extern bool GuiLoadGraph(BridgeCFGraphList* graph, duint addr);
extern duint GuiGraphAt(duint addr);
extern void GuiUpdateGraphView();
extern void GuiDisableLog();
extern void GuiEnableLog();
extern void GuiAddFavouriteTool(const char* name, const char* description);
extern void GuiAddFavouriteCommand(const char* name, const char* shortcut);
extern void GuiSetFavouriteToolShortcut(const char* name, const char* shortcut);
extern void GuiFoldDisassembly(duint startAddress, duint length);
extern void GuiSelectInMemoryMap(duint addr);
extern void GuiGetActiveView(ACTIVEVIEW* activeView);
extern void GuiAddInfoLine(const char* infoLine);
extern void GuiProcessEvents();
extern void* GuiTypeAddNode(void* parent, const TYPEDESCRIPTOR* type);
extern bool GuiTypeClear();
extern void GuiUpdateTypeWidget();
extern void GuiCloseApplication();
extern void GuiFlushLog();
extern void GuiReferenceAddCommand(const char* title, const char* command);
extern void GuiUpdateTraceBrowser();
extern void GuiOpenTraceFile(const char* fileName);
extern void GuiInvalidateSymbolSource(duint base);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif // _BRIDGEMAIN_H_
