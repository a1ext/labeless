/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"

#include <strsafe.h>

#include "labeless_olly.h"
#include "sdk/Plugin.h"

#include "labeless.h"
#include "../common/version.h"

#if FOFF_BUILD == 1
#pragma comment(linker, "/export:__FOFF_Plugindata=__ODBG_Plugindata")
#pragma comment(linker, "/export:__FOFF_Plugininit=__ODBG_Plugininit")
#pragma comment(linker, "/export:__FOFF_Pluginmainloop=__ODBG_Pluginmainloop")
#pragma comment(linker, "/export:__FOFF_Pluginsaveudd=__ODBG_Pluginsaveudd")
#pragma comment(linker, "/export:__FOFF_Pluginuddrecord=__ODBG_Pluginuddrecord")
#pragma comment(linker, "/export:__FOFF_Pluginmenu=__ODBG_Pluginmenu")
#pragma comment(linker, "/export:__FOFF_Pluginaction=__ODBG_Pluginaction")
#pragma comment(linker, "/export:__FOFF_Pluginshortcut=__ODBG_Pluginshortcut")
#pragma comment(linker, "/export:__FOFF_Pluginreset=__ODBG_Pluginreset")
#pragma comment(linker, "/export:__FOFF_Pluginclose=__ODBG_Pluginclose")
#pragma comment(linker, "/export:__FOFF_Plugindestroy=__ODBG_Plugindestroy")
#pragma comment(linker, "/export:__FOFF_Paused=__ODBG_Paused")
#pragma comment(linker, "/export:__FOFF_Pausedex=__ODBG_Pausedex")
#pragma comment(linker, "/export:__FOFF_Plugincmd=__ODBG_Plugincmd")
#endif // FOFF_BUILD

static HWND hwMain = nullptr;

#define UDD_ID 0xC4A86984;

enum MenuAction
{
	MA_Enable,
	MA_Disable,
	MA_SetPort,
	MA_SetIncommingIPFiltering,
	MA_ShowConfig,
	MA_About
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (DLL_PROCESS_ATTACH == ul_reason_for_call)
	{
		srand(GetTickCount());

		Labeless::instance().setHInstance(hModule);
	}
	return TRUE;
}

extc int  _export cdecl ODBG_Plugindata(char shortname[32])
{
	strcpy_s(shortname,  32, "Labeless");
	return PLUGIN_VERSION;
}

extc int  _export cdecl ODBG_Plugininit(int ollydbgversion, HWND hw, ulong* features)
{
	if (ollydbgversion < PLUGIN_VERSION)
		return -1;

	hwMain = hw;

	Labeless& ll = Labeless::instance();
	ll.setPort(WORD(Pluginreadintfromini(ll.hInstance(), "port", ll.port())));
	char buff[MAX_PATH] = {};
	Pluginreadstringfromini(ll.hInstance(), "filer_ip", buff, "");
	ll.setFilterIP(buff);

	if (!Labeless::instance().init())
	{
		log_r("labeless::init() failed.");
		return -1;
	}
	Addtolist(0, 0, "Labeless");
	Addtolist(0, -1, "  Written by Aliaksandr Trafimchuk");

	return 0;
}

extc void _export cdecl ODBG_Pluginmainloop(DEBUG_EVENT* debugevent)
{

}

extc void _export cdecl ODBG_Pluginsaveudd(t_module* pmod, int ismainmodule)
{

}

extc int  _export cdecl ODBG_Pluginuddrecord(t_module* pmod, int ismainmodule, ulong tag, ulong size, void* data)
{
	return 0;
}

extc int  _export cdecl ODBG_Pluginmenu(int origin, char data[4096], void *item)
{
	switch (origin)
	{
	case PM_MAIN:
		strcpy_s(data, 4096,
			"0 Enable,1 Disable|"
			"2 Set listening port...,"
			"3 Set IP Filter...|"
			"4 Show config|"
			"5 &About");
		return 1;
	default:
		return 0;
	}
}

extc void _export cdecl ODBG_Pluginaction(int origin, int action, void* item)
{
	switch (origin)
	{
	case PM_MAIN:
		switch (static_cast<MenuAction>(action))
		{
		case MA_Enable:
			Labeless::instance().startServer();
			break;
		case MA_Disable:
			Labeless::instance().stopServer();
			break;
		case MA_SetPort:
			Labeless::instance().onSetPortRequested();
			break;
		case MA_SetIncommingIPFiltering:
			Labeless::instance().onSetIPFilter();
			break;
		case MA_ShowConfig:
			do {
				char buff[MAX_PATH] = {};
				StringCchPrintf(buff, MAX_PATH, "Listening port: %u\r\nAllowed connect from IP: %s",
					Labeless::instance().port(),
					Labeless::instance().filterIP().empty() ? "any" : Labeless::instance().filterIP().c_str());
				MessageBox(hwMain, buff, "Labeless config", MB_ICONINFORMATION);
			} while (0);
			break;
		case MA_About:
			do {
				char buff[MAX_PATH] = {};
				StringCchPrintf(buff,
					MAX_PATH,
					"Labeless\nBuilt [%s]\nVersion: %s",
					Labeless::instance().lastChangeTimestamp().c_str(),
					LABELESS_VER_STR);
				MessageBox(hwMain, buff, "About", MB_ICONINFORMATION);
				break;
			} while (0);
			break;
		}
		break;
	default:
		break;
	}
}

extc int  _export cdecl ODBG_Pluginshortcut(int origin, int ctrl, int alt, int shift, int key, void* item)
{
	return 0;
}

extc void _export cdecl ODBG_Pluginreset()
{

}

extc int  _export cdecl ODBG_Pluginclose()
{
	return 0;
}

extc void _export cdecl ODBG_Plugindestroy()
{
	Labeless::instance().destroy();
}

extc int  _export cdecl ODBG_Paused(int reason, t_reg* reg)
{
	return 0;
}

extc int  _export cdecl ODBG_Pausedex(int reasonex, int dummy, t_reg* reg, DEBUG_EVENT* debugevent)
{
	return 0;
}

extc int  _export cdecl ODBG_Plugincmd(int reason, t_reg* reg, char* cmd)
{
	return 0;
}
