/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <memory>
#include <QSettings>

QT_FORWARD_DECLARE_CLASS(QSharedMemory)


enum GlobalSettingsKey
{
	GSK_Unknown = 0,
	GSK_ColorScheme = 1,
	GSK_Templates = 2,
	GSK_PrevEnteredOllyHosts = 3,
	GSK_PrevSelectedOllyHost = 4,
	GSK_AnalyzePEHeader = 5,
	GSK_DefaultExternSegSize = 6, // Deprecated, don't use it
	GSK_PostProcessFixCallJumps = 7,
	GSK_LightPalette = 8,
	GSK_DarkPalette = 9,
	GSK_OverwriteWarning = 10,
};

typedef std::shared_ptr<QSettings> QSettingsPtr;
class GlobalSettingsManger;

class ScopedSettings
{
	Q_DISABLE_COPY(ScopedSettings)
public:
	QSettingsPtr settings;

	ScopedSettings(GlobalSettingsManger* manager, QSettingsPtr pSettings);
	~ScopedSettings();

private:
	GlobalSettingsManger* const pManager;
};

class GlobalSettingsManger : public QObject
{
	Q_OBJECT

	GlobalSettingsManger();
public:
	~GlobalSettingsManger();

	static GlobalSettingsManger& instance();

	std::shared_ptr<ScopedSettings> get();

	QVariant value(GlobalSettingsKey key, const QVariant& defaultValue = QVariant(), const QString& group = QString::null);
	bool setValue(GlobalSettingsKey key, const QVariant& value, const QString& group = QString::null);

	void detach();

private:
	void lock();
	void unlock();

	static QString keyToString(GlobalSettingsKey key);

private:
	std::shared_ptr<QSharedMemory>	m_Memory;
	QSettingsPtr					m_Settings;

	friend class ScopedSettings;
};

