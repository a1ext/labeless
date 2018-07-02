#ifndef _SCRIPTAPI_MEMORY_H
#define _SCRIPTAPI_MEMORY_H

#include "_scriptapi.h"

namespace Script
{
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
        
        %rename(Memory_GetProtect) GetProtect;
        extern unsigned int GetProtect(duint addr, bool reserved = false, bool cache = true);
        
        %rename(Memory_GetBase) GetBase;
        extern duint GetBase(duint addr, bool reserved = false, bool cache = true);
        
        %rename(Memory_GetSize) GetSize;
        extern duint GetSize(duint addr, bool reserved = false, bool cache = true);

        
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
}; //Script

#endif //_SCRIPTAPI_MEMORY_H