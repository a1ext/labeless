#ifndef _SCRIPTAPI_MISC_H
#define _SCRIPTAPI_MISC_H

#include "_scriptapi.h"

namespace Script
{
    namespace Misc
    {
        extern bool ParseExpression(const char* expression, duint* value);
        extern duint RemoteGetProcAddress(const char* module, const char* api);
        extern duint ResolveLabel(const char* label);
        extern void* Alloc(duint size);
        extern void Free(void* ptr);
    }; //Misc
}; //Script

#endif //_SCRIPTAPI_MISC_H