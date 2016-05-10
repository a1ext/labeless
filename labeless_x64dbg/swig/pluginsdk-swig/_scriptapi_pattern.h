#ifndef _SCRIPTAPI_PATTERN_H
#define _SCRIPTAPI_PATTERN_H

#include "_scriptapi.h"

namespace Script
{
    namespace Pattern
    {
        extern duint Find(unsigned char* data, duint datasize, const char* pattern);
        extern duint FindMem(duint start, duint size, const char* pattern);
        extern void Write(unsigned char* data, duint datasize, const char* pattern);
        extern void WriteMem(duint start, duint size, const char* pattern);
        extern bool SearchAndReplace(unsigned char* data, duint datasize, const char* searchpattern, const char* replacepattern);
        extern bool SearchAndReplaceMem(duint start, duint size, const char* searchpattern, const char* replacepattern);
    };
};

#endif //_SCRIPTAPI_FIND_H