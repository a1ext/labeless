#ifndef _SCRIPTAPI_DEBUG_H
#define _SCRIPTAPI_DEBUG_H

#include "_scriptapi.h"

namespace Script
{
    namespace Debug
    {
        enum HardwareType
        {
            HardwareAccess,
            HardwareWrite,
            HardwareExecute
        };

        extern void Wait();
        extern void Run();
        extern void Pause();
        extern void Stop();
        extern void StepIn();
        extern void StepOver();
        extern void StepOut();
        extern bool SetBreakpoint(duint address);
        extern bool DeleteBreakpoint(duint address);
        extern bool SetHardwareBreakpoint(duint address, HardwareType type = HardwareExecute);
        extern bool DeleteHardwareBreakpoint(duint address);
    }; //Debug
}; //Script

#endif //_SCRIPTAPI_DEBUG_H