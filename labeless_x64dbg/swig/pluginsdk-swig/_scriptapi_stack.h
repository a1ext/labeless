#ifndef _SCRIPTAPI_STACK_H
#define _SCRIPTAPI_STACK_H

#include "_scriptapi.h"

namespace Script
{
    namespace Stack
    {
        %rename(Stack_Pop) Pop;
        extern duint Pop();

        %rename(Stack_Push) Push;
        extern duint Push(duint value); //returns the previous top, equal to Peek(1)

        %rename(Stack_Peek) Peek;
        extern duint Peek(int offset = 0); //offset is in multiples of Register::Size(), for easy x32/x64 portability
    }; //Stack
}; //Script

#endif //_SCRIPTAPI_STACK_H