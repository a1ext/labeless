#ifndef _SCRIPTAPI_MODULE_H
#define _SCRIPTAPI_MODULE_H

#include "_scriptapi.h"

namespace Script
{
    namespace Module
    {
        struct ModuleInfo
        {
            duint base;
            duint size;
            duint entry;
            int sectionCount;
            char name[MAX_MODULE_SIZE];
            char path[MAX_PATH];
        };

        struct ModuleSectionInfo
        {
            duint addr;
            duint size;
            char name[MAX_SECTION_SIZE * 5];
        };

        %rename(Module_InfoFromAddr) InfoFromAddr;
        extern bool InfoFromAddr(duint addr, ModuleInfo* info);
        
        %rename(Module_InfoFromName) InfoFromName;
        extern bool InfoFromName(const char* name, ModuleInfo* info);
        
        %rename(Module_BaseFromAddr) BaseFromAddr;
        extern duint BaseFromAddr(duint addr);
        
        %rename(Module_BaseFromName) BaseFromName;
        extern duint BaseFromName(const char* name);
        
        %rename(Module_SizeFromAddr) SizeFromAddr;
        extern duint SizeFromAddr(duint addr);
        
        %rename(Module_SizeFromName) SizeFromName;
        extern duint SizeFromName(const char* name);
        
        %rename(Module_NameFromAddr) NameFromAddr;
        %pybuffer_string(char* name)
        extern bool NameFromAddr(duint addr, char* name); //name[MAX_MODULE_SIZE]
        
        %rename(Module_PathFromAddr) PathFromAddr;
        %pybuffer_string(char* path)
        extern bool PathFromAddr(duint addr, char* path); //path[MAX_PATH]
        
        %rename(Module_PathFromName) PathFromName;
        %pybuffer_string(char* path)
        extern bool PathFromName(const char* name, char* path); //path[MAX_PATH]
        
        %rename(Module_EntryFromAddr) EntryFromAddr;
        extern duint EntryFromAddr(duint addr);
        
        %rename(Module_EntryFromName) EntryFromName;
        extern duint EntryFromName(const char* name);
        
        %rename(Module_SectionCountFromAddr) SectionCountFromAddr;
        extern int SectionCountFromAddr(duint addr);
        
        %rename(Module_SectionCountFromName) SectionCountFromName;
        extern int SectionCountFromName(const char* name);
        
        %rename(Module_SectionFromAddr) SectionFromAddr;
        extern bool SectionFromAddr(duint addr, int number, ModuleSectionInfo* section);
        
        %rename(Module_SectionFromName) SectionFromName;
        extern bool SectionFromName(const char* name, int number, ModuleSectionInfo* section);
        
        %rename(Module_SectionListFromAddr) SectionListFromAddr;
        extern bool SectionListFromAddr(duint addr, ListInfo* list);
        
        %rename(Module_SectionListFromName) SectionListFromName;
        extern bool SectionListFromName(const char* name, ListInfo* list);
        
        %rename(Module_GetMainModuleInfo) GetMainModuleInfo;
        extern bool GetMainModuleInfo(ModuleInfo* info);
        
        %rename(Module_GetMainModuleBase) GetMainModuleBase;
        extern duint GetMainModuleBase();
        
        %rename(Module_GetMainModuleSize) GetMainModuleSize;
        extern duint GetMainModuleSize();
        
        %rename(Module_GetMainModuleEntry) GetMainModuleEntry;
        extern duint GetMainModuleEntry();
        
        %rename(Module_GetMainModuleSectionCount) GetMainModuleSectionCount;
        extern int GetMainModuleSectionCount();
        
        %rename(Module_GetMainModuleName) GetMainModuleName;
        %pybuffer_string(char* name)
        extern bool GetMainModuleName(char* name); //name[MAX_MODULE_SIZE]
        
        %rename(Module_GetMainModulePath) GetMainModulePath;
        %pybuffer_string(char* path)
        extern bool GetMainModulePath(char* path); //path[MAX_PATH]
        
        %rename(Module_GetMainModuleSectionList) GetMainModuleSectionList;
        extern bool GetMainModuleSectionList(ListInfo* list); //caller has the responsibility to free the list
        
        %rename(Module_GetList) GetList;
        extern bool GetList(ListInfo* list); //caller has the responsibility to free the list
    }; //Module
}; //Script

#endif //_SCRIPTAPI_MODULE_H