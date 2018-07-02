#ifndef _SCRIPTAPI_LABEL_H
#define _SCRIPTAPI_LABEL_H

#include "_scriptapi.h"

namespace Script
{
    namespace Label
    {
        struct LabelInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            char text[MAX_LABEL_SIZE];
            bool manual;
        };

        %rename(Label_Set) Set;
        extern bool Set(duint addr, const char* text, bool manual = false);
        
        %rename(Label_SetByLabelInfo) Set;
        extern bool Set(const LabelInfo* info);
        
        %rename(Label_FromString) FromString;
        extern bool FromString(const char* label, duint* addr);
        
        %rename(Label_Get) Get;
        extern bool Get(duint addr, char* text); //text[MAX_LABEL_SIZE]
        
        %rename(Label_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, LabelInfo* info);
        
        %rename(Label_Delete) Delete;
        extern bool Delete(duint addr);
        
        %rename(Label_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);
        
        %rename(Label_Clear) Clear;
        extern void Clear();
        
        %rename(Label_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Label
}; //Script

#endif //_SCRIPTAPI_LABEL_H