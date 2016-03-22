#ifndef _LABELESS_X64DBG_H
#define _LABELESS_X64DBG_H

#include "types.h"

#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif //DLL_EXPORT

//superglobal variables
extern HINSTANCE g_hInstance;
extern int g_pluginHandle;
extern HWND g_hwndDlg;
extern int g_hMenu;
extern int g_hMenuDisasm;
extern int g_hMenuDump;
extern int g_hMenuStack;

extern PROCESS_INFORMATION g_pi;


#ifdef __cplusplus
extern "C"
{
#endif

DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct);
DLL_EXPORT bool plugstop();
DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct);

#ifdef __cplusplus
}
#endif

#endif //_LABELESS_X64DBG_H
