#ifndef _SCRIPTAPI_COMMENT_H
#define _SCRIPTAPI_COMMENT_H

#include "_scriptapi.h"

namespace Script
{
    namespace Comment
    {
        struct CommentInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            char text[MAX_LABEL_SIZE];
            bool manual;
        };

        %rename(Comment_Set) Set;
        extern bool Set(duint addr, const char* text, bool manual = false);

        %rename(Comment_ByCommentInfo) Set;
        extern bool Set(const CommentInfo* info);

        %rename(Comment_Get) Get;
        extern bool Get(duint addr, char* text); //text[MAX_COMMENT_SIZE]

        %rename(Comment_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, CommentInfo* info);

        %rename(Comment_Delete) Delete;
        extern bool Delete(duint addr);

        %rename(Comment_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);

        %rename(Comment_Clear) Clear;
        extern void Clear();
        
        %rename(Comment_GetList) GetList;
        extern bool GetList(ListOf(CommentInfo) list); //caller has the responsibility to free the list
    }; //Comment
}; //Script

#endif //_SCRIPTAPI_COMMENT_H