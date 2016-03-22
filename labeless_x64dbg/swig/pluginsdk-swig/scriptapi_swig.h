#include "_scriptapi.h"
#include "bridelist.h"



namespace Script
{
    namespace Assembler
    {
        %rename(Assembler_Assemble) Assemble;
        %pybuffer_string(unsigned char* dest)
        extern bool Assemble(duint addr, unsigned char* dest, int* size, const char* instruction); //dest[16]

        %rename(Assembler_AssembleEx) AssembleEx;
        %pybuffer_string(unsigned char* dest)
        %pybuffer_string(char* error)
        extern bool AssembleEx(duint addr, unsigned char* dest, int* size, const char* instruction, char* error); //dest[16], error[MAX_ERROR_SIZE]

        %rename(Assembler_AssembleMem) AssembleMem;
        extern bool AssembleMem(duint addr, const char* instruction);

        %rename(Assembler_AssembleMemEx) AssembleMemEx;
        %pybuffer_string(char* error)
        extern bool AssembleMemEx(duint addr, const char* instruction, int* size, char* error, bool fillnop); //error[MAX_ERROR_SIZE]
    }; //Assembler

    namespace Bookmark
    {
        struct BookmarkInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            bool manual;
        };

        %rename(Bookmark_Set) Set;
        extern bool Set(duint addr, bool manual = false);
        %rename(Bookmark_SetByBookmarkInfo) Set;
        extern bool Set(const BookmarkInfo* info);
        %rename(Bookmark_Get) Get;
        extern bool Get(duint addr);
        %rename(Bookmark_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, BookmarkInfo* info);
        %rename(Bookmark_Delete) Delete;
        extern bool Delete(duint addr);
        %rename(Bookmark_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);
        %rename(Bookmark_Clear) Clear;
        extern void Clear();
        %rename(Bookmark_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list

        
    }; //Bookmark

    namespace Comment
    {
        struct CommentInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            char text[MAX_LABEL_SIZE];
            bool manual;
        };

        %rename(Comment_Set) Set;
        extern bool Set(duint addr, const char* text, bool manual = false);
        %rename(Comment_ByCommentInfo) Set;
        extern bool Set(const CommentInfo* info);
        %rename(Comment_Get) Get;
        %pybuffer_string(char* text)
        extern bool Get(duint addr, char* text); //text[MAX_COMMENT_SIZE]
        %rename(Comment_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, CommentInfo* info);
        %rename(Comment_Delete) Delete;
        extern bool Delete(duint addr);
        %rename(Comment_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);
        %rename(Comment_Clear) Clear;
        extern void Clear();
        %rename(Comment_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Comment

    namespace Debug
    {
        enum HardwareType
        {
            HardwareAccess,
            HardwareWrite,
            HardwareExecute
        };

        %rename(Debug_Wait) Wait;
        extern void Wait();
        %rename(Debug_Run) Run;
        extern void Run();
        %rename(Debug_Pause) Pause;
        extern void Pause();
        %rename(Debug_Stop) Stop;
        extern void Stop();
        %rename(Debug_StepIn) StepIn;
        extern void StepIn();
        %rename(Debug_StepOver) StepOver;
        extern void StepOver();
        %rename(Debug_StepOut) StepOut;
        extern void StepOut();
        %rename(Debug_SetBreakpoint) SetBreakpoint;
        extern bool SetBreakpoint(duint address);
        %rename(Debug_DeleteBreakpoint) DeleteBreakpoint;
        extern bool DeleteBreakpoint(duint address);
        %rename(Debug_SetHardwareBreakpoint) SetHardwareBreakpoint;
        extern bool SetHardwareBreakpoint(duint address, HardwareType type = HardwareExecute);
        %rename(Debug_DeleteHardwareBreakpoint) DeleteHardwareBreakpoint;
        extern bool DeleteHardwareBreakpoint(duint address);
    }; //Debug

    namespace Flag
    {
        enum FlagEnum
        {
            ZF,
            OF,
            CF,
            PF,
            SF,
            TF,
            AF,
            DF,
            IF
        };

        %rename(Flag_Get) Get;
        extern bool Get(FlagEnum flag);
        %rename(Flag_Set) Set;
        extern bool Set(FlagEnum flag, bool value);

        %rename(Flag_GetZF) GetZF;
        extern bool GetZF();
        %rename(Flag_SetZF) SetZF;
        extern bool SetZF(bool value);
        %rename(Flag_GetOF) GetOF;
        extern bool GetOF();
        %rename(Flag_SetOF) SetOF;
        extern bool SetOF(bool value);
        %rename(Flag_GetCF) GetCF;
        extern bool GetCF();
        %rename(Flag_SetCF) SetCF;
        extern bool SetCF(bool value);
        %rename(Flag_GetPF) GetPF;
        extern bool GetPF();
        %rename(Flag_SetPF) SetPF;
        extern bool SetPF(bool value);
        %rename(Flag_GetSF) GetSF;
        extern bool GetSF();
        %rename(Flag_SetSF) SetSF;
        extern bool SetSF(bool value);
        %rename(Flag_GetTF) GetTF;
        extern bool GetTF();
        %rename(Flag_SetTF) SetTF;
        extern bool SetTF(bool value);
        %rename(Flag_GetAF) GetAF;
        extern bool GetAF();
        %rename(Flag_SetAF) SetAF;
        extern bool SetAF(bool value);
        %rename(Flag_GetDF) GetDF;
        extern bool GetDF();
        %rename(Flag_SetDF) SetDF;
        extern bool SetDF(bool value);
        %rename(Flag_GetIF) GetIF;
        extern bool GetIF();
        %rename(Flag_SetIF) SetIF;
        extern bool SetIF(bool value);
    };

    namespace Function
    {
        struct FunctionInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rvaStart;
            duint rvaEnd;
            bool manual;
            duint instructioncount;
        };

        %rename(Function_Add) Add;
        extern bool Add(duint start, duint end, bool manual, duint instructionCount = 0);
        %rename(Function_AddByFuncInfo) Add;
        extern bool Add(const FunctionInfo* info);
        %rename(Function_Get) Get;
        extern bool Get(duint addr, duint* start = nullptr, duint* end = nullptr, duint* instructionCount = nullptr);
        %rename(Function_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, FunctionInfo* info);
        %rename(Function_Overlaps) Overlaps;
        extern bool Overlaps(duint start, duint end);
        %rename(Function_Delete) Delete;
        extern bool Delete(duint address);
        %rename(Function_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);
        %rename(Function_Clear) Clear;
        extern void Clear();
        %rename(Function_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Function

    namespace Gui
    {
        namespace Disassembly
        {
            %rename(Gui_Disassembly_SelectionGet) SelectionGet;
            extern bool SelectionGet(duint* start, duint* end);
            %rename(Gui_Disassembly_SelectionSet) SelectionSet;
            extern bool SelectionSet(duint start, duint end);
            %rename(Gui_Disassembly_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
            %rename(Gui_Disassembly_SelectionGetEnd) SelectionGetEnd;
            extern duint SelectionGetEnd();
        }; //Disassembly

        namespace Dump
        {
            %rename(Gui_Dump_SelectionGet) SelectionGet;
            extern bool SelectionGet(duint* start, duint* end);
            %rename(Gui_Dump_SelectionSet) SelectionSet;
            extern bool SelectionSet(duint start, duint end);
            %rename(Gui_Dump_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
            %rename(Gui_Dump_SelectionGetEnd) SelectionGetEnd;
            extern duint SelectionGetEnd();
        }; //Dump

        namespace Stack
        {
            %rename(Gui_Stack_SelectionGet) SelectionGet;
            extern bool SelectionGet(duint* start, duint* end);
            %rename(Gui_Stack_SelectionSet) SelectionSet;
            extern bool SelectionSet(duint start, duint end);
            %rename(Gui_Stack_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
            %rename(Gui_Stack_SelectionGetEnd) SelectionGetEnd;
            extern duint SelectionGetEnd();
        }; //Stack

        enum Window
        {
            DisassemblyWindow,
            DumpWindow,
            StackWindow
        };

        %rename(Gui_SelectionGet) SelectionGet;
        extern bool SelectionGet(Window window, duint* start, duint* end);
        %rename(Gui_SelectionSet) SelectionSet;
        extern bool SelectionSet(Window window, duint start, duint end);
        %rename(Gui_SelectionGetStart) SelectionGetStart;
        extern duint SelectionGetStart(Window window);
        %rename(Gui_SelectionGetEnd) SelectionGetEnd;
        extern duint SelectionGetEnd(Window window);
        %rename(Gui_Message) Message;
        extern void Message(const char* message);
        %rename(Gui_MessageYesNo) MessageYesNo;
        extern bool MessageYesNo(const char* message);
        %rename(Gui_InputLine) InputLine;
        %pybuffer_string(char* text)
        extern bool InputLine(const char* title, char* text); //text[GUI_MAX_LINE_SIZE]
        %rename(Gui_InputValue) InputValue;
        extern bool InputValue(const char* title, duint* value);
        %rename(Gui_Refresh) Refresh;
        extern void Refresh();
        %rename(Gui_AddQWidgetTab) AddQWidgetTab;
        extern void AddQWidgetTab(void* qWidget);
        %rename(Gui_ShowQWidgetTab) ShowQWidgetTab;
        extern void ShowQWidgetTab(void* qWidget);
        %rename(Gui_CloseQWidgetTab) CloseQWidgetTab;
        extern void CloseQWidgetTab(void* qWidget);

    }; //Gui

    namespace Label
    {
        struct LabelInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            char text[MAX_LABEL_SIZE];
            bool manual;
        };

        %rename(Label_Set) Set;
        extern bool Set(duint addr, const char* text, bool manual = false);
        %rename(Label_SetByLabelInfo) Set;
        extern bool Set(const LabelInfo* info);
        %rename(Label_FromString) FromString;
        extern bool FromString(const char* label, duint* addr);
        %rename(Label_Get) Get;
        %pybuffer_string(char* text)
        extern bool Get(duint addr, char* text); //text[MAX_LABEL_SIZE]
        %rename(Label_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, LabelInfo* info);
        %rename(Label_Delete) Delete;
        extern bool Delete(duint addr);
        %rename(Label_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);
        %rename(Label_Clear) Clear;
        extern void Clear();
        %rename(Label_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Label

    namespace Memory
    {
        %rename(Memory_Read) Read;
        extern bool Read(duint addr, void* data, duint size, duint* sizeRead);
        %rename(Memory_Write) Write;
        extern bool Write(duint addr, const void* data, duint size, duint* sizeWritten);
        %rename(Memory_IsValidPtr) IsValidPtr;
        extern bool IsValidPtr(duint addr);
        %rename(Memory_RemoteAlloc) RemoteAlloc;
        extern duint RemoteAlloc(duint addr, duint size);
        %rename(Memory_RemoteFree) RemoteFree;
        extern bool RemoteFree(duint addr);

        %rename(Memory_ReadByte) ReadByte;
        extern unsigned char ReadByte(duint addr);
        %rename(Memory_WriteByte) WriteByte;
        extern bool WriteByte(duint addr, unsigned char data);
        %rename(Memory_ReadWord) ReadWord;
        extern unsigned short ReadWord(duint addr);
        %rename(Memory_WriteWord) WriteWord;
        extern bool WriteWord(duint addr, unsigned short data);
        %rename(Memory_ReadDword) ReadDword;
        extern unsigned int ReadDword(duint addr);
        %rename(Memory_WriteDword) WriteDword;
        extern bool WriteDword(duint addr, unsigned int data);
        %rename(Memory_ReadQword) ReadQword;
        extern unsigned long long ReadQword(duint addr);
        %rename(Memory_WriteQword) WriteQword;
        extern bool WriteQword(duint addr, unsigned long long data);
        %rename(Memory_ReadPtr) ReadPtr;
        extern duint ReadPtr(duint addr);
        %rename(Memory_WritePtr) WritePtr;
        extern bool WritePtr(duint addr, duint data);
    }; //Memory

    namespace Misc
    {
        %rename(Misc_ParseExpression) ParseExpression;
        extern bool ParseExpression(const char* expression, duint* value);
        %rename(Misc_RemoteGetProcAddress) RemoteGetProcAddress;
        extern duint RemoteGetProcAddress(const char* module, const char* api);
        %rename(Misc_ResolveLabel) ResolveLabel;
        extern duint ResolveLabel(const char* label);
        %rename(Misc_Alloc) Alloc;
        extern void* Alloc(duint size);
        %rename(Misc_Free) Free;
        extern void Free(void* ptr);
    }; //Misc

    namespace Module
    {
        struct ModuleInfo
        {
            duint base;
            duint size;
            duint entry;
            int sectionCount;
            char name[MAX_MODULE_SIZE];
            char path[MAX_PATH];
        };

        struct ModuleSectionInfo
        {
            duint addr;
            duint size;
            char name[MAX_SECTION_SIZE * 5];
        };

        %rename(Module_InfoFromAddr) InfoFromAddr;
        extern bool InfoFromAddr(duint addr, ModuleInfo* info);
        %rename(Module_InfoFromName) InfoFromName;
        extern bool InfoFromName(const char* name, ModuleInfo* info);
        %rename(Module_BaseFromAddr) BaseFromAddr;
        extern duint BaseFromAddr(duint addr);
        %rename(Module_BaseFromName) BaseFromName;
        extern duint BaseFromName(const char* name);
        %rename(Module_SizeFromAddr) SizeFromAddr;
        extern duint SizeFromAddr(duint addr);
        %rename(Module_SizeFromName) SizeFromName;
        extern duint SizeFromName(const char* name);
        %rename(Module_NameFromAddr) NameFromAddr;
        %pybuffer_string(char* name)
        extern bool NameFromAddr(duint addr, char* name); //name[MAX_MODULE_SIZE]
        %rename(Module_PathFromAddr) PathFromAddr;
        %pybuffer_string(char* path)
        extern bool PathFromAddr(duint addr, char* path); //path[MAX_PATH]
        %rename(Module_PathFromName) PathFromName;
        %pybuffer_string(char* path)
        extern bool PathFromName(const char* name, char* path); //path[MAX_PATH]
        %rename(Module_EntryFromAddr) EntryFromAddr;
        extern duint EntryFromAddr(duint addr);
        %rename(Module_EntryFromName) EntryFromName;
        extern duint EntryFromName(const char* name);
        %rename(Module_SectionCountFromAddr) SectionCountFromAddr;
        extern int SectionCountFromAddr(duint addr);
        %rename(Module_SectionCountFromName) SectionCountFromName;
        extern int SectionCountFromName(const char* name);
        %rename(Module_SectionFromAddr) SectionFromAddr;
        extern bool SectionFromAddr(duint addr, int number, ModuleSectionInfo* section);
        %rename(Module_SectionFromName) SectionFromName;
        extern bool SectionFromName(const char* name, int number, ModuleSectionInfo* section);
        %rename(Module_SectionListFromAddr) SectionListFromAddr;
        extern bool SectionListFromAddr(duint addr, ListInfo* list);
        %rename(Module_SectionListFromName) SectionListFromName;
        extern bool SectionListFromName(const char* name, ListInfo* list);
        %rename(Module_GetMainModuleInfo) GetMainModuleInfo;
        extern bool GetMainModuleInfo(ModuleInfo* info);
        %rename(Module_GetMainModuleBase) GetMainModuleBase;
        extern duint GetMainModuleBase();
        %rename(Module_GetMainModuleSize) GetMainModuleSize;
        extern duint GetMainModuleSize();
        %rename(Module_GetMainModuleEntry) GetMainModuleEntry;
        extern duint GetMainModuleEntry();
        %rename(Module_GetMainModuleSectionCount) GetMainModuleSectionCount;
        extern int GetMainModuleSectionCount();
        %rename(Module_GetMainModuleName) GetMainModuleName;
        %pybuffer_string(char* name)
        extern bool GetMainModuleName(char* name); //name[MAX_MODULE_SIZE]
        %rename(Module_GetMainModulePath) GetMainModulePath;
        %pybuffer_string(char* path)
        extern bool GetMainModulePath(char* path); //path[MAX_PATH]
        %rename(Module_GetMainModuleSectionList) GetMainModuleSectionList;
        extern bool GetMainModuleSectionList(ListInfo* list); //caller has the responsibility to free the list
        %rename(Module_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Module

    namespace Pattern
    {
        %rename(Pattern_Find) Find;
        %pybuffer_string(unsigned char* data)
        extern duint Find(unsigned char* data, duint datasize, const char* pattern);
        %rename(Pattern_FindMem) FindMem;
        extern duint FindMem(duint start, duint size, const char* pattern);
        %rename(Pattern_Write) Write;
        %pybuffer_string(unsigned char* data)
        extern void Write(unsigned char* data, duint datasize, const char* pattern);
        %rename(Pattern_WriteMem) WriteMem;
        extern void WriteMem(duint start, duint size, const char* pattern);
        %rename(Pattern_SearchAndReplace) SearchAndReplace;
        %pybuffer_string(unsigned char* data)
        extern bool SearchAndReplace(unsigned char* data, duint datasize, const char* searchpattern, const char* replacepattern);
        %rename(Pattern_SearchAndReplaceMem) SearchAndReplaceMem;
        extern bool SearchAndReplaceMem(duint start, duint size, const char* searchpattern, const char* replacepattern);
    };

    namespace Register
    {
        enum RegisterEnum
        {
            DR0,
            DR1,
            DR2,
            DR3,
            DR6,
            DR7,

            EAX,
            AX,
            AH,
            AL,
            EBX,
            BX,
            BH,
            BL,
            ECX,
            CX,
            CH,
            CL,
            EDX,
            DX,
            DH,
            DL,
            EDI,
            DI,
            ESI,
            SI,
            EBP,
            BP,
            ESP,
            SP,
            EIP,

#ifdef _WIN64
            RAX,
            RBX,
            RCX,
            RDX,
            RSI,
            SIL,
            RDI,
            DIL,
            RBP,
            BPL,
            RSP,
            SPL,
            RIP,
            R8,
            R8D,
            R8W,
            R8B,
            R9,
            R9D,
            R9W,
            R9B,
            R10,
            R10D,
            R10W,
            R10B,
            R11,
            R11D,
            R11W,
            R11B,
            R12,
            R12D,
            R12W,
            R12B,
            R13,
            R13D,
            R13W,
            R13B,
            R14,
            R14D,
            R14W,
            R14B,
            R15,
            R15D,
            R15W,
            R15B,
#endif //_WIN64

            CIP,
            CSP,
        }; //RegisterEnum

        %rename(Register_Get) Get;
        extern duint Get(RegisterEnum reg);
        %rename(Register_Set) Set;
        extern bool Set(RegisterEnum reg, duint value);
        %rename(Register_Size) Size;
        extern int Size(); //gets architecture register size in bytes

        %rename(Register_GetDR0) GetDR0;
        extern duint GetDR0();
        %rename(Register_SetDR0) SetDR0;
        extern bool SetDR0(duint value);
        %rename(Register_GetDR1) GetDR1;
        extern duint GetDR1();
        %rename(Register_SetDR1) SetDR1;
        extern bool SetDR1(duint value);
        %rename(Register_GetDR2) GetDR2;
        extern duint GetDR2();
        %rename(Register_SetDR2) SetDR2;
        extern bool SetDR2(duint value);
        %rename(Register_GetDR3) GetDR3;
        extern duint GetDR3();
        %rename(Register_SetDR3) SetDR3;
        extern bool SetDR3(duint value);
        %rename(Register_GetDR6) GetDR6;
        extern duint GetDR6();
        %rename(Register_SetDR6) SetDR6;
        extern bool SetDR6(duint value);
        %rename(Register_GetDR7) GetDR7;
        extern duint GetDR7();
        %rename(Register_SetDR7) SetDR7;
        extern bool SetDR7(duint value);

        %rename(Register_GetEAX) GetEAX;
        extern unsigned int GetEAX();
        %rename(Register_SetEAX) SetEAX;
        extern bool SetEAX(unsigned int value);
        %rename(Register_GetAX) GetAX;
        extern unsigned short GetAX();
        %rename(Register_SetAX) SetAX;
        extern bool SetAX(unsigned short value);
        %rename(Register_GetAH) GetAH;
        extern unsigned char GetAH();
        %rename(Register_SetAH) SetAH;
        extern bool SetAH(unsigned char value);
        %rename(Register_GetAL) GetAL;
        extern unsigned char GetAL();
        %rename(Register_SetAL) SetAL;
        extern bool SetAL(unsigned char value);
        %rename(Register_GetEBX) GetEBX;
        extern unsigned int GetEBX();
        %rename(Register_SetEBX) SetEBX;
        extern bool SetEBX(unsigned int value);
        %rename(Register_GetBX) GetBX;
        extern unsigned short GetBX();
        %rename(Register_SetBX) SetBX;
        extern bool SetBX(unsigned short value);
        %rename(Register_GetBH) GetBH;
        extern unsigned char GetBH();
        %rename(Register_SetBH) SetBH;
        extern bool SetBH(unsigned char value);
        %rename(Register_GetBL) GetBL;
        extern unsigned char GetBL();
        %rename(Register_SetBL) SetBL;
        extern bool SetBL(unsigned char value);
        %rename(Register_GetECX) GetECX;
        extern unsigned int GetECX();
        %rename(Register_SetECX) SetECX;
        extern bool SetECX(unsigned int value);
        %rename(Register_GetCX) GetCX;
        extern unsigned short GetCX();
        %rename(Register_SetCX) SetCX;
        extern bool SetCX(unsigned short value);
        %rename(Register_GetCH) GetCH;
        extern unsigned char GetCH();
        %rename(Register_SetCH) SetCH;
        extern bool SetCH(unsigned char value);
        %rename(Register_GetCL) GetCL;
        extern unsigned char GetCL();
        %rename(Register_SetCL) SetCL;
        extern bool SetCL(unsigned char value);
        %rename(Register_GetEDX) GetEDX;
        extern unsigned int GetEDX();
        %rename(Register_SetEDX) SetEDX;
        extern bool SetEDX(unsigned int value);
        %rename(Register_GetDX) GetDX;
        extern unsigned short GetDX();
        %rename(Register_SetDX) SetDX;
        extern bool SetDX(unsigned short value);
        %rename(Register_GetDH) GetDH;
        extern unsigned char GetDH();
        %rename(Register_SetDH) SetDH;
        extern bool SetDH(unsigned char value);
        %rename(Register_GetDL) GetDL;
        extern unsigned char GetDL();
        %rename(Register_SetDL) SetDL;
        extern bool SetDL(unsigned char value);
        %rename(Register_GetEDI) GetEDI;
        extern unsigned int GetEDI();
        %rename(Register_SetEDI) SetEDI;
        extern bool SetEDI(unsigned int value);
        %rename(Register_GetDI) GetDI;
        extern unsigned short GetDI();
        %rename(Register_SetDI) SetDI;
        extern bool SetDI(unsigned short value);
        %rename(Register_GetESI) GetESI;
        extern unsigned int GetESI();
        %rename(Register_SetESI) SetESI;
        extern bool SetESI(unsigned int value);
        %rename(Register_GetSI) GetSI;
        extern unsigned short GetSI();
        %rename(Register_SetSI) SetSI;
        extern bool SetSI(unsigned short value);
        %rename(Register_GetEBP) GetEBP;
        extern unsigned int GetEBP();
        %rename(Register_SetEBP) SetEBP;
        extern bool SetEBP(unsigned int value);
        %rename(Register_GetBP) GetBP;
        extern unsigned short GetBP();
        %rename(Register_SetBP) SetBP;
        extern bool SetBP(unsigned short value);
        %rename(Register_GetESP) GetESP;
        extern unsigned int GetESP();
        %rename(Register_SetESP) SetESP;
        extern bool SetESP(unsigned int value);
        %rename(Register_GetSP) GetSP;
        extern unsigned short GetSP();
        %rename(Register_SetSP) SetSP;
        extern bool SetSP(unsigned short value);
        %rename(Register_GetEIP) GetEIP;
        extern unsigned int GetEIP();
        %rename(Register_SetEIP) SetEIP;
        extern bool SetEIP(unsigned int value);

#ifdef _WIN64
        %rename(Register_GetRAX) GetRAX;
        extern unsigned long long GetRAX();
        %rename(Register_SetRAX) SetRAX;
        extern bool SetRAX(unsigned long long value);
        %rename(Register_GetRBX) GetRBX;
        extern unsigned long long GetRBX();
        %rename(Register_SetRBX) SetRBX;
        extern bool SetRBX(unsigned long long value);
        %rename(Register_GetRCX) GetRCX;
        extern unsigned long long GetRCX();
        %rename(Register_SetRCX) SetRCX;
        extern bool SetRCX(unsigned long long value);
        %rename(Register_GetRDX) GetRDX;
        extern unsigned long long GetRDX();
        %rename(Register_SetRDX) SetRDX;
        extern bool SetRDX(unsigned long long value);
        %rename(Register_GetRSI) GetRSI;
        extern unsigned long long GetRSI();
        %rename(Register_SetRSI) SetRSI;
        extern bool SetRSI(unsigned long long value);
        %rename(Register_GetSIL) GetSIL;
        extern unsigned char GetSIL();
        %rename(Register_SetSIL) SetSIL;
        extern bool SetSIL(unsigned char value);
        %rename(Register_GetRDI) GetRDI;
        extern unsigned long long GetRDI();
        %rename(Register_SetRDI) SetRDI;
        extern bool SetRDI(unsigned long long value);
        %rename(Register_GetDIL) GetDIL;
        extern unsigned char GetDIL();
        %rename(Register_SetDIL) SetDIL;
        extern bool SetDIL(unsigned char value);
        %rename(Register_GetRBP) GetRBP;
        extern unsigned long long GetRBP();
        %rename(Register_SetRBP) SetRBP;
        extern bool SetRBP(unsigned long long value);
        %rename(Register_GetBPL) GetBPL;
        extern unsigned char GetBPL();
        %rename(Register_SetBPL) SetBPL;
        extern bool SetBPL(unsigned char value);
        %rename(Register_GetRSP) GetRSP;
        extern unsigned long long GetRSP();
        %rename(Register_SetRSP) SetRSP;
        extern bool SetRSP(unsigned long long value);
        %rename(Register_GetSPL) GetSPL;
        extern unsigned char GetSPL();
        %rename(Register_SetSPL) SetSPL;
        extern bool SetSPL(unsigned char value);
        %rename(Register_GetRIP) GetRIP;
        extern unsigned long long GetRIP();
        %rename(Register_SetRIP) SetRIP;
        extern bool SetRIP(unsigned long long value);
        %rename(Register_GetR8) GetR8;
        extern unsigned long long GetR8();
        %rename(Register_SetR8) SetR8;
        extern bool SetR8(unsigned long long value);
        %rename(Register_GetR8D) GetR8D;
        extern unsigned int GetR8D();
        %rename(Register_SetR8D) SetR8D;
        extern bool SetR8D(unsigned int value);
        %rename(Register_GetR8W) GetR8W;
        extern unsigned short GetR8W();
        %rename(Register_SetR8W) SetR8W;
        extern bool SetR8W(unsigned short value);
        %rename(Register_GetR8B) GetR8B;
        extern unsigned char GetR8B();
        %rename(Register_SetR8B) SetR8B;
        extern bool SetR8B(unsigned char value);
        %rename(Register_GetR9) GetR9;
        extern unsigned long long GetR9();
        %rename(Register_SetR9) SetR9;
        extern bool SetR9(unsigned long long value);
        %rename(Register_GetR9D) GetR9D;
        extern unsigned int GetR9D();
        %rename(Register_SetR9D) SetR9D;
        extern bool SetR9D(unsigned int value);
        %rename(Register_GetR9W) GetR9W;
        extern unsigned short GetR9W();
        %rename(Register_SetR9W) SetR9W;
        extern bool SetR9W(unsigned short value);
        %rename(Register_GetR9B) GetR9B;
        extern unsigned char GetR9B();
        %rename(Register_SetR9B) SetR9B;
        extern bool SetR9B(unsigned char value);
        %rename(Register_GetR10) GetR10;
        extern unsigned long long GetR10();
        %rename(Register_SetR10) SetR10;
        extern bool SetR10(unsigned long long value);
        %rename(Register_GetR10D) GetR10D;
        extern unsigned int GetR10D();
        %rename(Register_SetR10D) SetR10D;
        extern bool SetR10D(unsigned int value);
        %rename(Register_GetR10W) GetR10W;
        extern unsigned short GetR10W();
        %rename(Register_SetR10W) SetR10W;
        extern bool SetR10W(unsigned short value);
        %rename(Register_GetR10B) GetR10B;
        extern unsigned char GetR10B();
        %rename(Register_SetR10B) SetR10B;
        extern bool SetR10B(unsigned char value);
        %rename(Register_GetR11) GetR11;
        extern unsigned long long GetR11();
        %rename(Register_SetR11) SetR11;
        extern bool SetR11(unsigned long long value);
        %rename(Register_GetR11D) GetR11D;
        extern unsigned int GetR11D();
        %rename(Register_SetR11D) SetR11D;
        extern bool SetR11D(unsigned int value);
        %rename(Register_GetR11W) GetR11W;
        extern unsigned short GetR11W();
        %rename(Register_SetR11W) SetR11W;
        extern bool SetR11W(unsigned short value);
        %rename(Register_GetR11B) GetR11B;
        extern unsigned char GetR11B();
        %rename(Register_SetR11B) SetR11B;
        extern bool SetR11B(unsigned char value);
        %rename(Register_GetR12) GetR12;
        extern unsigned long long GetR12();
        %rename(Register_SetR12) SetR12;
        extern bool SetR12(unsigned long long value);
        %rename(Register_GetR12D) GetR12D;
        extern unsigned int GetR12D();
        %rename(Register_SetR12D) SetR12D;
        extern bool SetR12D(unsigned int value);
        %rename(Register_GetR12W) GetR12W;
        extern unsigned short GetR12W();
        %rename(Register_SetR12W) SetR12W;
        extern bool SetR12W(unsigned short value);
        %rename(Register_GetR12B) GetR12B;
        extern unsigned char GetR12B();
        %rename(Register_SetR12B) SetR12B;
        extern bool SetR12B(unsigned char value);
        %rename(Register_GetR13) GetR13;
        extern unsigned long long GetR13();
        %rename(Register_SetR13) SetR13;
        extern bool SetR13(unsigned long long value);
        %rename(Register_GetR13D) GetR13D;
        extern unsigned int GetR13D();
        %rename(Register_SetR13D) SetR13D;
        extern bool SetR13D(unsigned int value);
        %rename(Register_GetR13W) GetR13W;
        extern unsigned short GetR13W();
        %rename(Register_SetR13W) SetR13W;
        extern bool SetR13W(unsigned short value);
        %rename(Register_GetR13B) GetR13B;
        extern unsigned char GetR13B();
        %rename(Register_SetR13B) SetR13B;
        extern bool SetR13B(unsigned char value);
        %rename(Register_GetR14) GetR14;
        extern unsigned long long GetR14();
        %rename(Register_SetR14) SetR14;
        extern bool SetR14(unsigned long long value);
        %rename(Register_GetR14D) GetR14D;
        extern unsigned int GetR14D();
        %rename(Register_SetR14D) SetR14D;
        extern bool SetR14D(unsigned int value);
        %rename(Register_GetR14W) GetR14W;
        extern unsigned short GetR14W();
        %rename(Register_SetR14W) SetR14W;
        extern bool SetR14W(unsigned short value);
        %rename(Register_GetR14B) GetR14B;
        extern unsigned char GetR14B();
        %rename(Register_SetR14B) SetR14B;
        extern bool SetR14B(unsigned char value);
        %rename(Register_GetR15) GetR15;
        extern unsigned long long GetR15();
        %rename(Register_SetR15) SetR15;
        extern bool SetR15(unsigned long long value);
        %rename(Register_GetR15D) GetR15D;
        extern unsigned int GetR15D();
        %rename(Register_SetR15D) SetR15D;
        extern bool SetR15D(unsigned int value);
        %rename(Register_GetR15W) GetR15W;
        extern unsigned short GetR15W();
        %rename(Register_SetR15W) SetR15W;
        extern bool SetR15W(unsigned short value);
        %rename(Register_GetR15B) GetR15B;
        extern unsigned char GetR15B();
        %rename(Register_SetR15B) SetR15B;
        extern bool SetR15B(unsigned char value);
#endif //_WIN64

        %rename(Register_GetCIP) GetCIP;
        extern duint GetCIP();
        %rename(Register_SetCIP) SetCIP;
        extern bool SetCIP(duint value);
        %rename(Register_GetCSP) GetCSP;
        extern duint GetCSP();
        %rename(Register_SetCSP) SetCSP;
        extern bool SetCSP(duint value);
    }; //Register

    namespace Stack
    {
        %rename(Stack_Pop) Pop;
        extern duint Pop();
        %rename(Stack_Push) Push;
        extern duint Push(duint value); //returns the previous top, equal to Peek(1)
        %rename(Stack_Peek) Peek;
        extern duint Peek(int offset = 0); //offset is in multiples of Register::Size(), for easy x32/x64 portability
    }; //Stack

    namespace Symbol
    {
        enum SymbolType
        {
            Function,
            Import,
            Export
        };

        struct SymbolInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            char name[MAX_LABEL_SIZE];
            bool manual;
            SymbolType type;
        };

        %rename(Symbol_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Symbol
}; //Script
