#ifndef _SCRIPTAPI_ASSEMBLER_H
#define _SCRIPTAPI_ASSEMBLER_H

#include "_scriptapi.h"

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
}; //Script

#endif //_SCRIPTAPI_ASSEMBLER_H