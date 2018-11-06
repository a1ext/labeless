/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <QColor>
#include <QMap>
#include <QTextCharFormat>

#include "palette.h"

enum PythonPaletteType : int
{
	PPT_Light,
	PPT_Dark
};



QTextCharFormat getTextCharFormat(PythonPaletteEntryType t, const PythonPalette& pp);

class PythonPaletteManager
{
	Q_DISABLE_COPY(PythonPaletteManager)
	PythonPaletteManager();
public:
	static PythonPalette getDefaultLightPalette();
	static PythonPalette getDefaultDarkPalette();

	static PythonPaletteManager& instance();

	inline const PythonPalette& palette() const { return m_Specs; }
	inline PythonPalette& lightPalette() { return m_SpecsLight; }
	inline PythonPalette& darkPalette() { return m_SpecsDark; }

	inline const FormatSpec formatSpec(PythonPaletteEntryType t) const { return m_Specs.palette[t]; }
	inline FormatSpec& formatSpec(PythonPaletteEntryType t) { return m_Specs.palette[t]; }

	QTextCharFormat getTextCharFormat(PythonPaletteEntryType t) const;
	void switchScheme(bool isDark);

	void storeSettings();
	void loadSettings();

private:
	void updateWithModifier(QTextCharFormat& fmt, FormatSpec::Modifiers mods) const;

private:
	PythonPalette& m_Specs;
	PythonPalette m_SpecsLight;
	PythonPalette m_SpecsDark;
};
