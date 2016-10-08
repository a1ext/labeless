#pragma once

#include <QColor>
#include "types.h"

struct FormatSpec
{
	enum Modifiers
	{
		MOD_Normal = 0,
		MOD_Bold = 1 << 0,
		MOD_Italic = 1 << 1
	};

	QColor color;
	Modifiers modifiers;
};


enum PythonPaletteEntryType FORCE_ENUM_SIZE_INT
{
	PPET_Unknown,
	PPET_Keyword,
	PPET_Operator,
	PPET_Reserved,
	PPET_Brace,
	PPET_Defclass,
	PPET_String,
	PPET_String2,
	PPET_Comment,
	PPET_Self,
	PPET_Number
};

struct PythonPalette
{
	typedef QMap<PythonPaletteEntryType, FormatSpec> PyPaletteMap;
	PyPaletteMap palette;
	QString mainFont;
	int mainFontPointSize;
	int tabWidth;
};

