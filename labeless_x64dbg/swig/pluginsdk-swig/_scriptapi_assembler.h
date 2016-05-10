#ifndef _SCRIPTAPI_ASSEMBLER_H
#define _SCRIPTAPI_ASSEMBLER_H

#include "_scriptapi.h"

namespace Script
{
    namespace Assembler
    {
        extern bool Assemble(duint addr, unsigned char* dest, int* size, const char* instruction); //dest[16]
        extern bool AssembleEx(duint addr, unsigned char* dest, int* size, const char* instruction, char* error); //dest[16], error[MAX_ERROR_SIZE]
        extern bool AssembleMem(duint addr, const char* instruction);
        extern bool AssembleMemEx(duint addr, const char* instruction, int* size, char* error, bool fillnop); //error[MAX_ERROR_SIZE]
    }; //Assembler
}; //Script

#endif //_SCRIPTAPI_ASSEMBLER_H