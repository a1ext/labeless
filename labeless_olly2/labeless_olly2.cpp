/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"

#include <strsafe.h>
#include <intsafe.h>

#include "labeless_olly2.h"
#include "sdk/Plugin.h"

#include "labeless.h"
#include "../common/version.h"

namespace {

enum MenuAction
{
	MA_Enable,
	MA_Disable,
	MA_SetPort,
	MA_SetIncommingIPFiltering,
	MA_ShowConfig,
	MA_About
};


static t_menu kMainMenu [] =
{
	{
		L"Enable",
		L"Enable Labeless.",
		K_NONE, handle_menu, NULL, MA_Enable
	},
	{
		L"Disable",
		L"Disable Labeless.",
		K_NONE, handle_menu, NULL, MA_Disable
	},
	{
		L"|Set listening port...",
		L"Sets the listenimg port.",
		K_NONE, handle_menu, NULL, MA_SetPort
	},
	{
		L"Set IP Filter...",
		L"Sets IP filter.",
		K_NONE, handle_menu, NULL, MA_SetIncommingIPFiltering
	},
	{
		L"|Show config",
		L"Shows configuration dialog.",
		K_NONE, handle_menu, NULL, MA_ShowConfig
	},
	{
		L"|About",
		L"Fire the about messagebox.",
		K_NONE, handle_menu, NULL, MA_About
	},
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

static const std::wstring kSettingsFileName = L"labeless.ini";
static const std::wstring kSettingsNetSection = L"net";
static const std::wstring kSettingsLabelessPort = L"labeless-port";
static const std::wstring kSettingsLabelessFilterIP = L"labeless-filter-ip";


} // anonymous

static HWND hwMain = nullptr;
static HINSTANCE g_hInstance;

#define UDD_ID 0xC4A86984;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (DLL_PROCESS_ATTACH == ul_reason_for_call)
	{
		g_hInstance = hModule;
		DisableThreadLibraryCalls(hModule);
	}
	return TRUE;
}

extc int __cdecl ODBG2_Plugininit(void)
{
	Labeless& ll = Labeless::instance();
	ll.setHInstance(g_hInstance);

	DWORD port = 0;
	Getfromini(const_cast<wchar_t*>(kSettingsFileName.c_str()),
		const_cast<wchar_t*>(kSettingsNetSection.c_str()),
		const_cast<wchar_t*>(kSettingsLabelessPort.c_str()),
		L"%u", &port);

	if (port == 0 || port > SHORT_MAX)
		port = ll.port();
	ll.setPort(WORD(port));

	wchar_t buff[MAX_PATH] = {};
	Getfromini(const_cast<wchar_t*>(kSettingsFileName.c_str()),
		const_cast<wchar_t*>(kSettingsNetSection.c_str()),
		const_cast<wchar_t*>(kSettingsLabelessFilterIP.c_str()),
		L"%s", buff);
	ll.setFilterIP(buff);

	if (!Labeless::instance().init())
	{
		log_r("labeless::init() failed.");
		return -1;
	}
	Addtolist(0, BLACK, L"Labeless");
	Addtolist(0, 2, L"  Written by Aliaksandr Trafimchuk");
	
	return 0;
}

extc int __cdecl ODBG2_Pluginquery(int ollydbgversion, ulong *features, wchar_t pluginname[SHORTNAME], wchar_t pluginversion[SHORTNAME])
{
	if (ollydbgversion < 201)
		return 0;
	wcscpy_s(pluginname, SHORTNAME, L"Labeless");
	wcscpy_s(pluginversion, SHORTNAME, LABELESS_VER_STR);
	hwMain = hwollymain;

	return PLUGIN_VERSION;
}

int handle_menu(t_table* pTable, wchar_t* pName, ulong index, int nMode)
{
	if (nMode == MENU_VERIFY)
		return MENU_NORMAL;
	if (nMode != MENU_EXECUTE)
		return MENU_ABSENT;

	switch (static_cast<MenuAction>(index))
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
			wchar_t buff[MAX_PATH] = {};
			StringCchPrintfW(buff, MAX_PATH, L"Listening port: %u\r\nAllowed connect from IP: %s",
				Labeless::instance().port(),
				Labeless::instance().filterIP().empty() ? L"any" : Labeless::instance().filterIP().c_str());
			MessageBoxW(hwMain, buff, L"Labeless config", MB_ICONINFORMATION);
		} while (0);
		break;
	case MA_About:
		do {
			wchar_t buff[MAX_PATH] = {};
			StringCchPrintfW(buff,
				MAX_PATH,
				L"Labeless\nBuilt [%s]\nVersion: %s",
				Labeless::instance().lastChangeTimestamp().c_str(),
				LABELESS_VER_STR);
			MessageBoxW(hwMain, buff, L"About", MB_ICONINFORMATION);
			break;
		} while (0);
		break;
	}

	return MENU_NOREDRAW;
}

extc t_menu* __cdecl ODBG2_Pluginmenu(wchar_t* type)
{
	if (wcscmp(type, PWM_MAIN) == 0)
		return kMainMenu;

	return NULL;
}

extc void cdecl ODBG2_Plugindestroy()
{
	Labeless::instance().destroy();
}

void storePort(WORD port)
{
	Writetoini(const_cast<wchar_t*>(kSettingsFileName.c_str()),
		const_cast<wchar_t*>(kSettingsNetSection.c_str()),
		const_cast<wchar_t*>(kSettingsLabelessPort.c_str()), L"%u", port);
}

void storeFilterIP(const std::wstring& filter)
{
	Writetoini(const_cast<wchar_t*>(kSettingsFileName.c_str()),
		const_cast<wchar_t*>(kSettingsNetSection.c_str()),
		const_cast<wchar_t*>(kSettingsLabelessFilterIP.c_str()), L"%s", filter.c_str());
}