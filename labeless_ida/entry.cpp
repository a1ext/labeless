/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"
#include "labeless_ida.h"

#include <loader.hpp>


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	Q_INIT_RESOURCE(res);
	DisableThreadLibraryCalls(hModule);
	return TRUE;
}

static void idaapi run(int)
{
	Labeless::instance().onSettingsRequested();
}

static int idaapi init()
{
	if (!is_idaq())
		return PLUGIN_SKIP;
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
	Labeless::instance().firstInit();
	return PLUGIN_KEEP;
}

void idaapi term()
{
	unhook_from_notification_point(HT_IDP, Labeless::idp_callback);
	unhook_from_notification_point(HT_UI, Labeless::ui_callback);
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
