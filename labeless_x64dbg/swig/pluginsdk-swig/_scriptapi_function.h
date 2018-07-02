#ifndef _SCRIPTAPI_FUNCTION_H
#define _SCRIPTAPI_FUNCTION_H

#include "_scriptapi.h"

namespace Script
{
    namespace Function
    {
        struct FunctionInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rvaStart;
            duint rvaEnd;
            bool manual;
            duint instructioncount;
        };

        %rename(Function_Add) Add;
        extern bool Add(duint start, duint end, bool manual, duint instructionCount = 0);
        
        %rename(Function_AddByFuncInfo) Add;
        extern bool Add(const FunctionInfo* info);
        
        %rename(Function_Get) Get;
        extern bool Get(duint addr, duint* start = nullptr, duint* end = nullptr, duint* instructionCount = nullptr);
        
        %rename(Function_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, FunctionInfo* info);
        
        %rename(Function_Overlaps) Overlaps;
        extern bool Overlaps(duint start, duint end);
        
        %rename(Function_Delete) Delete;
        extern bool Delete(duint address);
        
        %rename(Function_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end, bool deleteManual);
        
        %rename(Function_DeleteRangeAuto) DeleteRange;
        extern void DeleteRange(duint start, duint end);
        
        %rename(Function_Clear) Clear;
        extern void Clear();
        
        %rename(Function_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Function
}; //Script

#endif //_SCRIPTAPI_FUNCTION_H