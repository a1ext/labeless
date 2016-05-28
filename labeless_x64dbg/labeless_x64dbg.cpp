/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "labeless_x64dbg.h"
#include "labeless.h"
#include "../common/version.h"
#include "pluginsdk/_scriptapi_memory.h"
#include "types.h"

#include <tchar.h>
#include <strsafe.h>
#include <intsafe.h>
#include <TlHelp32.h>

#define plugin_name "Labeless"
#define plugin_version (LABELESS_VER_INT)

HINSTANCE g_hInstance = NULL;
int g_pluginHandle = 0;
HWND g_hwndDlg = NULL;
int g_hMenu = 0;
int g_hMenuDisasm = 0;
int g_hMenuDump = 0;
int g_hMenuStack = 0;

PROCESS_INFORMATION g_pi = {};


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


static const xstring kSettingsNetSection = _T("net");
static const xstring kSettingsLabelessPort = _T("labeless-port");
static const xstring kSettingsLabelessFilterIP = _T("labeless-filter-ip");

static const std::string kCommandAllocEx = "allocex";


bool cbCommandEnable(int argc, char* argv [])
{
	return Labeless::instance().startServer();
}

bool cbCommandDisable(int argc, char* argv [])
{
	Labeless::instance().stopServer();
	return true;
}

bool cbCommandSetListeningPort(int argc, char* argv [])
{
	Labeless::instance().onSetPortRequested();
	return true;
}

bool cbCommandSetIpFilter(int argc, char* argv [])
{
	Labeless::instance().onSetIPFilter();

	return true;
}

bool cbCommandShowConfig(int argc, char* argv [])
{
	char buff[MAX_PATH] = {};
	StringCchPrintf(buff, MAX_PATH, "Listening port: %u\r\nAllowed connect from IP: %s",
		Labeless::instance().port(),
		Labeless::instance().filterIP().empty() ? "any" : Labeless::instance().filterIP().c_str());
	MessageBox(g_hwndDlg, buff, "Labeless config", MB_ICONINFORMATION);
	return true;
}

bool cbCommandAbout(int argc, char* argv [])
{
	char buff[MAX_PATH] = {};
	StringCchPrintfA(buff,
		MAX_PATH,
		"Labeless %s\nBuilt [%s]\nVersion: %s",
#ifdef _WIN64
		"x64"
#else // _WIN64
		"x32"
#endif // _WIN64
		, Labeless::instance().lastChangeTimestamp().c_str(),
		LABELESS_VER_STR);
	MessageBox(g_hwndDlg, buff, "About", MB_ICONINFORMATION);
	return true;
}

void cbCreateProcessAndAttachNotif(CBTYPE cbType, void* callbackInfo)
{
	if (CB_CREATEPROCESS == cbType)
	{
		if (PLUG_CB_CREATEPROCESS* info = reinterpret_cast<PLUG_CB_CREATEPROCESS*>(callbackInfo))
			g_pi = *info->fdProcessInfo;
	}
}

static bool cbDebugAllocEx(int argc, char* argv [])
{
	if (argc < 2)
		return false;

	duint addr = 0;
	if (!DbgFunctions()->ValFromString(argv[1], &addr))
		return false;
	duint size = 0x1000;
	if (argc > 2 && !DbgFunctions()->ValFromString(argv[2], &size))
		return false;

	duint mem = (duint) Script::Memory::RemoteAlloc(addr, size);
	if (!mem)
		_plugin_logprintf("%s: VirtualAllocEx failed\n", kCommandAllocEx.c_str());
	else
		_plugin_logprintf("%s: %" PRIXPTR "\n", kCommandAllocEx.c_str(), mem);

	GuiUpdateMemoryView();

	return true;
}

extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
	switch (static_cast<MenuAction>(info->hEntry))
	{
	case MA_Enable:
		cbCommandEnable(0, nullptr);
		break;
	case MA_Disable:
		cbCommandDisable(0, nullptr);
		break;
	case MA_SetPort:
		cbCommandSetListeningPort(0, nullptr);
		break;
	case MA_SetIncommingIPFiltering:
		cbCommandSetIpFilter(0, nullptr);
		break;
	case MA_ShowConfig:
		cbCommandShowConfig(0, nullptr);
		break;
	case MA_About:
		cbCommandAbout(0, nullptr);
		break;
	}
}

} // anonymous

DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
	initStruct->pluginVersion = plugin_version;
	initStruct->sdkVersion = PLUG_SDKVERSION;
	strncpy_s(initStruct->pluginName, _countof(initStruct->pluginName), plugin_name, _countof(initStruct->pluginName));
	g_pluginHandle = initStruct->g_pluginHandle;

	if (!_plugin_registercommand(g_pluginHandle, kCommandAllocEx.c_str(), cbDebugAllocEx, true))
		log_r("error registering the \"%s\" command!", kCommandAllocEx.c_str());

	Labeless& ll = Labeless::instance();
	ll.setHInstance(g_hInstance);

	if (!ll.initPython())
	{
		log_r("Labeless::initPython() failed");
		return false;
	}

	std::string buff(MAX_SETTING_SIZE, '\0');

	if (BridgeSettingGet(kSettingsNetSection.c_str(), kSettingsLabelessPort.c_str(), &buff[0]))
	{
		DWORD port = 0;
		sscanf_s(buff.c_str(), "%u", &port);
		if (port == 0 || port > SHORT_MAX)
			port = ll.port();
		ll.setPort(WORD(port));
	}

	if (BridgeSettingGet(kSettingsNetSection.c_str(), kSettingsLabelessFilterIP.c_str(), &buff[0]))
	{
		ll.setFilterIP(buff);
	}

	_plugin_registercallback(g_pluginHandle, CB_CREATEPROCESS, cbCreateProcessAndAttachNotif);

	return true;
}

DLL_EXPORT bool plugstop()
{
	_plugin_unregistercommand(g_pluginHandle, kCommandAllocEx.c_str());

	_plugin_unregistercallback(g_pluginHandle, CB_CREATEPROCESS);
	_plugin_menuclear(g_hMenu);

	Labeless::instance().destroy();
	return true;
}

DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
	g_hwndDlg = setupStruct->hwndDlg;
	g_hMenu = setupStruct->g_hMenu;
	g_hMenuDisasm = setupStruct->g_hMenuDisasm;
	g_hMenuDump = setupStruct->g_hMenuDump;
	g_hMenuStack = setupStruct->g_hMenuStack;
	
	if (!Labeless::instance().init(setupStruct))
	{
		log_r("labeless::init() failed.");
		return;
	}

	_plugin_menuaddentry(g_hMenu, MA_Enable, "Enable");
	_plugin_menuaddentry(g_hMenu, MA_Disable, "Disable");
	_plugin_menuaddseparator(g_hMenu);
	_plugin_menuaddentry(g_hMenu, MA_SetPort, "Set listening port...");
	_plugin_menuaddentry(g_hMenu, MA_SetIncommingIPFiltering, "Set IP Filter...");
	_plugin_menuaddseparator(g_hMenu);
	_plugin_menuaddentry(g_hMenu, MA_ShowConfig, "Show config");
	_plugin_menuaddseparator(g_hMenu);
	_plugin_menuaddentry(g_hMenu, MA_About, "About");

	_plugin_logputs("Labeless\n");
	_plugin_logputs("  Written by Aliaksandr Trafimchuk\n");
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (DLL_PROCESS_ATTACH == fdwReason)
	{
		srand(GetTickCount());

		g_hInstance = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
	}
	return TRUE;
}

void storePort(WORD port)
{
	char buff[1024] = {};
	StringCchPrintfA(buff, _countof(buff), "%u", port);
	BridgeSettingSet(kSettingsNetSection.c_str(), kSettingsLabelessPort.c_str(), buff);
}

void storeFilterIP(const xstring& filter)
{
	BridgeSettingSet(kSettingsNetSection.c_str(), kSettingsLabelessFilterIP.c_str(), filter.c_str());
}