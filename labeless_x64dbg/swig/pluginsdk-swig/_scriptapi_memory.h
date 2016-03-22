#ifndef _SCRIPTAPI_MEMORY_H
#define _SCRIPTAPI_MEMORY_H

#include "_scriptapi.h"

namespace Script
{
    namespace Memory
    {
        extern bool Read(duint addr, void* data, duint size, duint* sizeRead);
        extern bool Write(duint addr, const void* data, duint size, duint* sizeWritten);
        extern bool IsValidPtr(duint addr);
        extern duint RemoteAlloc(duint addr, duint size);
        extern bool RemoteFree(duint addr);

        extern unsigned char ReadByte(duint addr);
        extern bool WriteByte(duint addr, unsigned char data);
        extern unsigned short ReadWord(duint addr);
        extern bool WriteWord(duint addr, unsigned short data);
        extern unsigned int ReadDword(duint addr);
        extern bool WriteDword(duint addr, unsigned int data);
        extern unsigned long long ReadQword(duint addr);
        extern bool WriteQword(duint addr, unsigned long long data);
        extern duint ReadPtr(duint addr);
        extern bool WritePtr(duint addr, duint data);
    }; //Memory
}; //Script

#endif //_SCRIPTAPI_MEMORY_H