/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "globalsettingsmanager.h"
#include "types.h"

#include <QSharedMemory>
#include <QMutex>

#include "labeless_ida.h"

namespace {

static const QString kGlobalSettingsLockName = "labeless-settings-lock";
static const QString kSettingsOrg = "AlexWMF";
static const QString kSettingsAppName = "Labeless";

} // anonymous

GlobalSettingsManger::GlobalSettingsManger()
	: QObject(&Labeless::instance())
	, m_Memory(std::make_shared<QSharedMemory>(kGlobalSettingsLockName))
	, m_Settings(std::make_shared<QSettings>(kSettingsOrg, kSettingsAppName))
{
	if (!m_Memory->attach() && !m_Memory->create(10))
	{
		msg("Labeless: %s: failed to create global settings lock\n", __FUNCTION__);
		return;
	}
}

GlobalSettingsManger::~GlobalSettingsManger()
{
}

QVariant GlobalSettingsManger::value(GlobalSettingsKey key, const QVariant& defaultValue, const QString& group)
{
	const auto name = keyToString(key);
	if (name.isEmpty())
		return defaultValue;

	auto ss = get();
	if (!ss)
		return defaultValue;

	if (!group.isEmpty())
		ss->settings->beginGroup(group);

	const QVariant rv = ss->settings->value(name, defaultValue);

	if (!group.isEmpty())
		ss->settings->endGroup();
	return rv;
}

bool GlobalSettingsManger::setValue(GlobalSettingsKey key, const QVariant& value, const QString& group)
{
	const auto name = keyToString(key);
	if (name.isEmpty())
		return false;
	auto ss = get();
	if (!ss)
		return false;

	if (!group.isEmpty())
		ss->settings->beginGroup(group);

	ss->settings->setValue(name, value);

	if (!group.isEmpty())
		ss->settings->endGroup();
	return true;
}

GlobalSettingsManger& GlobalSettingsManger::instance()
{
	static QPointer<GlobalSettingsManger> self = new GlobalSettingsManger;
	return *self;
}

void GlobalSettingsManger::lock()
{
	if (m_Memory)
		m_Memory->lock();
}

void GlobalSettingsManger::unlock()
{
	if (m_Memory && m_Memory->isAttached())
		m_Memory->unlock();
}

std::shared_ptr<ScopedSettings> GlobalSettingsManger::get()
{
	if (m_Settings)
		return std::make_shared<ScopedSettings>(this, m_Settings);
	return std::shared_ptr<ScopedSettings>();
}

QString GlobalSettingsManger::keyToString(GlobalSettingsKey key)
{	static QHash<GlobalSettingsKey, QString> names;
	static QMutex namesLock;
	QMutexLocker locker(&namesLock);
	if (names.isEmpty())
	{
		static const struct {
			GlobalSettingsKey k;
			QString name;
		} kNames[] = {
			{ GSK_ColorScheme, "color_scheme" },
			{ GSK_Templates, "templates" },
			{ GSK_PrevEnteredOllyHosts, "prev_hosts" },
			{ GSK_PrevSelectedOllyHost, "prev_selected_olly_host" },
			{ GSK_AnalyzePEHeader, "analyse_pe_header" },
			{ GSK_DefaultExternSegSize, "default_extern_seg_size" },
			{ GSK_PostProcessFixCallJumps, "postprocess_fix_call_jumps_inside_another_instr" },
			{ GSK_LightPalette, "light_palette" },
			{ GSK_DarkPalette, "dark_palette" },
			{ GSK_OverwriteWarning, "overwrite_warning" },
		};
		for (unsigned i = 0; i < _countof(kNames); ++i)
			names[kNames[i].k] = kNames[i].name;
	}
	
	return names.value(key, QString::null);
}

void GlobalSettingsManger::detach()
{
	static bool detached = false;
	if (!detached && m_Memory && m_Memory->isAttached())
		m_Memory->detach();
	detached = true;
	m_Memory.reset();
	m_Settings.reset();
}

ScopedSettings::ScopedSettings(GlobalSettingsManger* manager, QSettingsPtr pSettings)
	: pManager(manager)
	, settings(pSettings)
{
	pManager->lock();
}

ScopedSettings::~ScopedSettings()
{
	settings->sync();
	pManager->unlock();
}
