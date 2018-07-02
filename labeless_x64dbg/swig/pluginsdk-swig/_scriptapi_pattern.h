#ifndef _SCRIPTAPI_PATTERN_H
#define _SCRIPTAPI_PATTERN_H

#include "_scriptapi.h"

namespace Script
{
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
};

#endif //_SCRIPTAPI_FIND_H