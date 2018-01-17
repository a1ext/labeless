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
#include "../common/version.h"

// IDA
#ifdef __NT__
#pragma warning(push)
#pragma warning(disable:4309 4244 4267)           // disable "truncation of constant value" warning from IDA SDK, conversion from 'ssize_t' to 'int', possible loss of data
#endif // __NT__
#include <loader.hpp>
#ifdef __NT__
#pragma warning(pop)
#endif // __NT__

// Qt
#include <QMetaType>
#include <QMessageBox>

namespace {

static const char kID[] = "by.a1ext.labeless";
static const char kLabelessPluginName[] = "Labeless";
static const char kAuthor[] = "Trafimchuk Aliaksandr";

} // anonymous

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

#if (IDA_SDK_VERSION < 700)
typedef void run_ret_t;
typedef int run_arg0_t;
#else
typedef bool run_ret_t;
typedef size_t run_arg0_t;
#endif

static run_ret_t idaapi run(run_arg0_t)
{
	if (!Labeless::instance().isEnabled())
		QMessageBox::information(nullptr, QObject::tr("Info"), QObject::tr("Open some database first"));
	else
		Labeless::instance().onSettingsRequested();
#if (IDA_SDK_VERSION >= 700)
	return true;
#endif // IDA_SDK_VERSION >= 700
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

	::addon_info_t addon_info;
	addon_info.cb = sizeof(::addon_info_t);
	addon_info.id = kID;
	addon_info.name = kLabelessPluginName;
	addon_info.producer = kAuthor;
	addon_info.version = LABELESS_VER_STR;
	addon_info.freeform = "";
	register_addon(&addon_info);

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
	kLabelessPluginName,  // the preferred short name of the plugin
	"Shift+Alt+E"         // the preferred hotkey to run the plugin
};
