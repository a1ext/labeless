#ifndef _SCRIPTAPI_BOOKMARK_H
#define _SCRIPTAPI_BOOKMARK_H

#include "_scriptapi.h"

namespace Script
{
    namespace Bookmark
    {
        struct BookmarkInfo
        {
            char mod[MAX_MODULE_SIZE];
            duint rva;
            bool manual;
        };

        %rename(Bookmark_Set) Set;
        extern bool Set(duint addr, bool manual = false);

        %rename(Bookmark_SetByBookmarkInfo) Set;
        extern bool Set(const BookmarkInfo* info);

        %rename(Bookmark_Get) Get;
        extern bool Get(duint addr);

        %rename(Bookmark_GetInfo) GetInfo;
        extern bool GetInfo(duint addr, BookmarkInfo* info);

        %rename(Bookmark_Delete) Delete;
        extern bool Delete(duint addr);

        %rename(Bookmark_DeleteRange) DeleteRange;
        extern void DeleteRange(duint start, duint end);

        %rename(Bookmark_Clear) Clear;
        extern void Clear();

        %rename(Bookmark_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Bookmark
}; //Script

#endif //_SCRIPTAPI_BOOKMARK_H