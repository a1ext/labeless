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
        
        %rename(Debug_DisableBreakpoint) DisableBreakpoint;
        extern bool DisableBreakpoint(duint address);
        
        %rename(Debug_SetHardwareBreakpoint) SetHardwareBreakpoint;
        extern bool SetHardwareBreakpoint(duint address, HardwareType type = HardwareExecute);
        
        %rename(Debug_DeleteHardwareBreakpoint) DeleteHardwareBreakpoint;
        extern bool DeleteHardwareBreakpoint(duint address);
    }; //Debug
}; //Script

#endif //_SCRIPTAPI_DEBUG_H