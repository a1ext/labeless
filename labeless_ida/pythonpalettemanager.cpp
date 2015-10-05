/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "pythonpalettemanager.h"
#include "globalsettingsmanager.h"

#include <pro.h>
#include <kernwin.hpp>
#include <QMutex>

namespace {

void updateWithModifier(QTextCharFormat& fmt, FormatSpec::Modifiers mods)
{
	if (mods & FormatSpec::MOD_Bold)
		fmt.setFontWeight(QFont::Bold);
	if (mods & FormatSpec::MOD_Italic)
		fmt.setFontItalic(true);
}

static const QString kColorSchemeLight = "light";
static const QString kPaletteColor = "color";
static const QString kPaletteModifiers = "mods";
static const QString kPalette = "palette";

bool checkFieldsExists(const QVariantMap& vm, const QStringList& names)
{
	foreach(const auto& name, names)
		if (!vm.contains(name))
			return false;
	return true;
}

bool toVariant(const PythonPalette& palette, QVariant& result)
{
	QVariantMap vm;
	for (auto it = palette.palette.constBegin(), end = palette.palette.constEnd(); it != end; ++it)
	{
		QVariantMap vmItem;
		vmItem[kPaletteColor] = it.value().color.name();
		vmItem[kPaletteModifiers] = it.value().modifiers;

		vm[QString::number(it.key())] = vmItem;
	}
	QVariantMap vmPalette;
	vmPalette[kPalette] = vm;
	result = vmPalette;
	return true;
}

bool fromVariant(const QVariant& vPalette, PythonPalette& result)
{
	if (vPalette.isNull() || !vPalette.isValid())
		return false;

	const QVariantMap vmPalette = vPalette.toMap();
	if (!checkFieldsExists(vmPalette, QStringList() << kPalette))
		return false;

	const QVariantMap vm = vmPalette[kPalette].toMap();
	static const QList<PythonPaletteEntryType> kValidEntryTypes = QList<PythonPaletteEntryType>() <<
		PPET_Keyword << PPET_Operator << PPET_Reserved << PPET_Brace << PPET_Defclass << PPET_String <<
		PPET_String2 << PPET_Comment << PPET_Self << PPET_Number;

	foreach(auto type, kValidEntryTypes)
	{
		const auto name = QString::number(type);
		if (vm.contains(name) && checkFieldsExists(vm[name].toMap(), QStringList() << kPaletteColor << kPaletteModifiers))
		{
			const auto vmSpec = vm[name].toMap();

			FormatSpec spec;
			spec.color = QColor(vmSpec[kPaletteColor].toString());
			spec.modifiers = static_cast<FormatSpec::Modifiers>(vmSpec[kPaletteModifiers].toInt());
			result.palette[type] = spec;
		}
	}
	return true;
}


} // anonymous

QTextCharFormat getTextCharFormat(PythonPaletteEntryType t, const PythonPalette& pp)
{
	QTextCharFormat rv;
	QColor c(Qt::black);

	auto it = pp.palette.find(t);
	if (it != pp.palette.end())
	{
		c = it.value().color;
		FormatSpec::Modifiers mods = it.value().modifiers;
		updateWithModifier(rv, mods);
	}
	rv.setForeground(c);
	return rv;
}

PythonPalette PythonPaletteManager::getDefaultLightPalette()
{
	static const struct {
		FormatSpec spec;
		PythonPaletteEntryType type;
	} kFormats[] = {
		{ { QColor(Qt::darkBlue),	FormatSpec::MOD_Bold },		PPET_Keyword },
		{ { QColor(Qt::black),		FormatSpec::MOD_Normal },	PPET_Operator },
		{ { QColor("#000080"),		FormatSpec::MOD_Normal },	PPET_Reserved },
		{ { QColor(Qt::darkGray),	FormatSpec::MOD_Normal },	PPET_Brace },
		{ { QColor(Qt::black),		FormatSpec::MOD_Bold },		PPET_Defclass },
		{ { QColor("#009800"),		FormatSpec::MOD_Normal },	PPET_String },
		{ { QColor("#009800"),		FormatSpec::MOD_Normal },	PPET_String2 },
		{ { QColor("#808080"),		FormatSpec::MOD_Italic },	PPET_Comment },
		{ { QColor("#94558d"),		FormatSpec::MOD_Normal },	PPET_Self },
		{ { QColor("#0000ff"),		FormatSpec::MOD_Normal },	PPET_Number }
	};

	static PythonPalette defaultPalette;
	static QMutex paletteLock;
	static bool initialized = false;

	QMutexLocker locker(&paletteLock);
	if (!initialized)
	{
		initialized = true;
		for (unsigned i = 0; i < _countof(kFormats); ++i)
			defaultPalette.palette[kFormats[i].type] = kFormats[i].spec;
	}
	
	return defaultPalette;
}

PythonPalette PythonPaletteManager::getDefaultDarkPalette()
{
	static const struct {
		FormatSpec spec;
		PythonPaletteEntryType type;
	} kFormats[] = {
		{ { QColor("#cc5e2d"), FormatSpec::MOD_Bold }, PPET_Keyword },
		{ { QColor(Qt::red), FormatSpec::MOD_Normal }, PPET_Operator },
		{ { QColor("#8888c6"), FormatSpec::MOD_Normal }, PPET_Reserved },
		{ { QColor(Qt::darkGray), FormatSpec::MOD_Normal }, PPET_Brace },
		{ { QColor("#a9b7c6"), FormatSpec::MOD_Bold }, PPET_Defclass },
		{ { QColor("#a5c261"), FormatSpec::MOD_Normal }, PPET_String },
		{ { QColor("#a5c261"), FormatSpec::MOD_Normal }, PPET_String2 },
		{ { QColor(Qt::darkGreen), FormatSpec::MOD_Italic }, PPET_Comment },
		{ { QColor("#94558d"), FormatSpec::MOD_Normal }, PPET_Self },
		{ { QColor("#6897bb"), FormatSpec::MOD_Normal }, PPET_Number }
	};
	
	static PythonPalette defaultPalette;
	static QMutex paletteLock;
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		for (unsigned i = 0; i < _countof(kFormats); ++i)
			defaultPalette.palette[kFormats[i].type] = kFormats[i].spec;
	}
	return defaultPalette;
}

PythonPaletteManager::PythonPaletteManager()
{
	m_SpecsLight = m_Specs = getDefaultLightPalette();
	m_SpecsDark = getDefaultDarkPalette();
	loadSettings();
}

PythonPaletteManager& PythonPaletteManager::instance()
{
	static PythonPaletteManager p;
	return p;
}

QTextCharFormat PythonPaletteManager::getTextCharFormat(PythonPaletteEntryType t) const
{
	return ::getTextCharFormat(t, m_Specs);
}

void PythonPaletteManager::switchScheme(bool isDark)
{
	m_Specs = isDark ? m_SpecsDark : m_SpecsLight;
}

void PythonPaletteManager::storeSettings()
{
	QVariant vLight;
	QVariant vDark;
	if (!toVariant(m_SpecsLight, vLight))
	{
		msg("%s: unable to serialize light PythonPalette\n", __FUNCTION__);
		return;
	}

	if (!toVariant(m_SpecsDark, vDark))
	{
		msg("%s: unable to serialize dark PythonPalette\n", __FUNCTION__);
		return;
	}

	GlobalSettingsManger::instance().setValue(GSK_LightPalette, vLight);
	GlobalSettingsManger::instance().setValue(GSK_DarkPalette, vDark);
}

void PythonPaletteManager::loadSettings()
{
	const QVariant vLight = GlobalSettingsManger::instance().value(GSK_LightPalette);
	const QVariant vDark = GlobalSettingsManger::instance().value(GSK_DarkPalette);
	const QString scheme = GlobalSettingsManger::instance().value(GSK_ColorScheme, kColorSchemeLight).toString().toLower();

	PythonPalette light = getDefaultLightPalette();
	if (fromVariant(vLight, light))
		m_SpecsLight = light;

	PythonPalette dark = getDefaultDarkPalette();
	if (fromVariant(vDark, dark))
		m_SpecsDark = dark;
	m_Specs = scheme == kColorSchemeLight ? m_SpecsLight : m_SpecsDark;
}
