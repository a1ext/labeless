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
#include "util/util_ida.h"

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
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMetaType>
#include <QMessageBox>

namespace {

static const char kID[] = "by.a1ext.labeless";
static const char kLabelessPluginName[] = "Labeless";
static const char kAuthor[] = "Trafimchuk Aliaksandr";
static const std::string kSyncAllNowActionName = "Labeless: Sync all now";

} // anonymous

#if (IDA_SDK_VERSION >= 800)
/*
 * Sync all handler IDA 8 and up
 **/
struct SyncAllNowActionHandler_t : public action_handler_t
{
	virtual int idaapi activate(action_activation_ctx_t*) override {
		if (Labeless::instance().isEnabled())
			QMetaObject::invokeMethod(&Labeless::instance(), "onSyncronizeAllRequested", Qt::QueuedConnection);
		return 1;
	}

	virtual action_state_t idaapi update(action_update_ctx_t*) override	{
		return Labeless::instance().isEnabled() ? AST_ENABLE : AST_DISABLE;
	}
};

struct plugin_ctx_t; // FWD
plugin_ctx_t* plugmod = nullptr;


struct plugin_ctx_t : public ::plugmod_t {
	SyncAllNowActionHandler_t SyncAllNowActionHandler;
	const action_desc_t sync_all_action;

	plugin_ctx_t()
		: sync_all_action( ACTION_DESC_LITERAL_PLUGMOD(
			kSyncAllNowActionName.c_str(),
			"Sync labels now",
			&SyncAllNowActionHandler,
			this,
			"Alt+Shift+R",
			NULL,
			-1)) {
	}

	virtual bool idaapi run(size_t) override {
		/*msg("%s: called\n", __FUNCTION__);
		auto mb = util::ida::findIDAMainWindow()->menuBar();
		for (const auto act : mb->actions()) {
			msg("%s: top action: %s\n", __FUNCTION__, act->text().toStdString().c_str());
		}
		msg("---\n");
		QList<QMenu*> found = mb->findChildren<QMenu*>();
		for (int i = 0; i < found.size(); ++i) {
			QMenu* m = found.at(i);
			if (m->title() == "Labeless") {
				mb->addAction(m->menuAction());
			}
			for (const auto action : m->actions()) {
				std::string name = action->objectName().toStdString() + " " + action->text().toStdString();
				msg("%s: %s\n", __FUNCTION__, name.c_str());
			}
			
		}*/
		if (!Labeless::instance().isEnabled())
			QMessageBox::information(nullptr, QObject::tr("Info"), QObject::tr("Open some database first"));
		else
			Labeless::instance().onSettingsRequested();
		return true;
	}


	virtual ~plugin_ctx_t() {
		msg("%s: term\n", __FUNCTION__);
		unhook_from_notification_point(HT_IDP, Labeless::idp_callback);
		unhook_from_notification_point(HT_UI, Labeless::ui_callback);
		unhook_from_notification_point(HT_IDB, Labeless::idb_callback);
		unregister_action(kSyncAllNowActionName.c_str());

		Labeless::instance().shutdown();
		/*if (plugmod)
			delete plugmod;*/
		plugmod = nullptr;
	}
	
};

#endif // IDA_SDK_VERSION >= 800


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

#if (IDA_SDK_VERSION < 800)
#	if (IDA_SDK_VERSION < 700)
typedef void run_ret_t;
typedef int run_arg0_t;
#	else // IDA_SDK_VERSION < 700
typedef bool run_ret_t;
typedef size_t run_arg0_t;
#	endif // IDA_SDK_VERSION < 700


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
#endif // IDA_SDK_VERSION < 800

#if (IDA_SDK_VERSION < 800)
typedef int init_ret_t;
#else // IDA_SDK_VERSION < 800
typedef plugmod_t* init_ret_t;
#endif // IDA_SDK_VERSION < 800

static init_ret_t idaapi init()
{
	init_ret_t rv = {};

	if (!is_idaq())
		return rv;

	Q_INIT_RESOURCE(res);
	if (!hook_to_notification_point(HT_IDP, Labeless::idp_callback))
	{
		msg("%s: hook_to_notification_point(HT_IDP) failed.", __FUNCTION__);
		return rv;
	}
	if (!hook_to_notification_point(HT_UI, Labeless::ui_callback))
	{
		msg("%s: hook_to_notification_point(HT_UI) failed.", __FUNCTION__);
		return rv;
	}
	if (!hook_to_notification_point(HT_IDB, Labeless::idb_callback))
	{
		msg("%s: hook_to_notification_point(HT_IDB) failed.", __FUNCTION__);
		return rv;
	}
#if (IDA_SDK_VERSION < 800)
	Labeless::instance().firstInit();
#else // IDA_SDK_VERSION < 800
	Labeless::instance().setUpMainWindowShowupHook();
#endif // IDA_SDK_VERSION < 800

	qRegisterMetaType<QSharedPointer<jedi::Request>>("QSharedPointer<jedi::Request>");
	qRegisterMetaType<QSharedPointer<jedi::Result>>("QSharedPointer<jedi::Result>");

#if (IDA_SDK_VERSION >= 800)
	if (!plugmod)
		plugmod = new plugin_ctx_t();

	if (!register_action(plugmod->sync_all_action))
		msg("%s: unable to register %s action\n", __FUNCTION__, kSyncAllNowActionName.c_str());
#endif // IDA_SDK_VERSION >= 800

	::addon_info_t addon_info;
	addon_info.cb = sizeof(::addon_info_t);
	addon_info.id = kID;
	addon_info.name = kLabelessPluginName;
	addon_info.producer = kAuthor;
	addon_info.version = LABELESS_VER_STR;
	addon_info.freeform = "";
	register_addon(&addon_info);

	// from run()
	// qstrvec_t acts;
	// get_registered_actions(&acts);
	// for (int i = 0; i < acts.size(); ++i)
	// 	msg("%s: action \"%s\"\n", __FUNCTION__, acts[i].c_str());
	
	/*if (!Labeless::instance().isEnabled())
		QMessageBox::information(nullptr, QObject::tr("Info"), QObject::tr("Open some database first"));
	else
		Labeless::instance().onSettingsRequested();*/
	// end from run()
	
	return 
#if (IDA_SDK_VERSION < 800)
		PLUGIN_KEEP
#else // IDA_SDK_VERSION < 800
		plugmod
#endif // IDA_SDK_VERSION < 800
		;
}

#if (IDA_SDK_VERSION < 800)
void idaapi term()
{
	unhook_from_notification_point(HT_IDP, Labeless::idp_callback);
	unhook_from_notification_point(HT_UI, Labeless::ui_callback);
	unhook_from_notification_point(HT_IDB, Labeless::idb_callback);
	unregister_action(kSyncAllNowActionName.c_str());

	Labeless::instance().shutdown();
}
#endif // IDA_SDK_VERSION < 800

plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	PLUGIN_FIX 
#if (IDA_SDK_VERSION >= 800)
		| PLUGIN_MOD | PLUGIN_MULTI
#endif // IDA_SDK_VERSION >= 800
	,
	init,                 // initialize
#if (IDA_SDK_VERSION < 800)
	term,                 // terminate. this pointer may be NULL.
	run,                  // invoke plugin
#else // IDA_SDK_VERSION < 800
	nullptr,              // terminate. this pointer may be NULL.
	nullptr,              // invoke plugin
#endif // IDA_SDK_VERSION < 800
	NULL,                 // long comment about the plugin
	NULL,                 // multiline help about the plugin
	kLabelessPluginName,  // the preferred short name of the plugin
	"Shift+Alt+E"         // the preferred hotkey to run the plugin
};
