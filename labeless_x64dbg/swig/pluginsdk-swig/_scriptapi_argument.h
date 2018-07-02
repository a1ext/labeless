#ifndef _SCRIPTAPI_ARGUMENT_H
#define _SCRIPTAPI_ARGUMENT_H

#include "_scriptapi.h"

namespace Script
{
    namespace Argument
    {
        struct ArgumentInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rvaStart;
            duint rvaEnd;
            bool manual;
            duint instructioncount;
        };

        %rename(Argument_Add) Add;
        extern bool Add(duint start, duint end, bool manual, duint instructionCount = 0);

        %rename(Argument_AddByArgumentInfo) Add;
        extern bool Add(const ArgumentInfo* info);

        %rename(Argument_Get) Get;
        extern bool Get(duint addr, duint* start = nullptr, duint* end = nullptr, duint* instructionCount = nullptr);

        %rename(Argument_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, ArgumentInfo* info);
        
        %rename(Argument_Overlaps) Overlaps;
        extern bool Overlaps(duint start, duint end);
        
        %rename(Argument_Delete) Delete;
        extern bool Delete(duint address);
        
        %rename(Argument_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end, bool deleteManual = false);
        
        %rename(Argument_Clear) Clear;
        extern void Clear();
        
        %rename(Argument_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Argument
}; //Script

#endif //_SCRIPTAPI_ARGUMENT_H