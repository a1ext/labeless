/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"
#include "labeless_ida.h"
#include "jedi.h"

// IDA
#include <loader.hpp>

// Qt
#include <QMetaType>

#ifdef __NT__
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
	}
	return TRUE;
}
#endif // __NT__

static void idaapi run(int)
{
	Labeless::instance().onSettingsRequested();
}

static int idaapi init()
{
	if (!is_idaq())
		return PLUGIN_SKIP;

	Q_INIT_RESOURCE(res);
	if (!hook_to_notification_point(HT_IDP, Labeless::idp_callback, nullptr))
	{
		msg("%s: hook_to_notification_point(HT_IDP) failed.", __FUNCTION__);
		return PLUGIN_SKIP;
	}
	if (!hook_to_notification_point(HT_UI, Labeless::ui_callback, nullptr))
	{
		msg("%s: hook_to_notification_point(HT_UI) failed.", __FUNCTION__);
		return PLUGIN_SKIP;
	}
	if (!hook_to_notification_point(HT_IDB, Labeless::idb_callback, nullptr))
	{
		msg("%s: hook_to_notification_point(HT_IDB) failed.", __FUNCTION__);
		return PLUGIN_SKIP;
	}
	Labeless::instance().firstInit();
	qRegisterMetaType<QSharedPointer<jedi::Request>>("QSharedPointer<jedi::Request>");
	qRegisterMetaType<QSharedPointer<jedi::Result>>("QSharedPointer<jedi::Result>");
	return PLUGIN_KEEP;
}

void idaapi term()
{
	unhook_from_notification_point(HT_IDP, Labeless::idp_callback);
	unhook_from_notification_point(HT_UI, Labeless::ui_callback);
	unhook_from_notification_point(HT_IDB, Labeless::idb_callback);
	Labeless::instance().shutdown();
}

plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	PLUGIN_FIX,
	init,                 // initialize
	term,                 // terminate. this pointer may be NULL.
	run,                  // invoke plugin
	NULL,                 // long comment about the plugin
	NULL,                 // multiline help about the plugin
	"Labeless",           // the preferred short name of the plugin
	"Shift+Alt+E"         // the preferred hotkey to run the plugin
};
