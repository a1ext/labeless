/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include "types.h"

// std
#include <string>

// Qt
#include <QString>

QT_FORWARD_DECLARE_CLASS(QMainWindow)


namespace util {
namespace ida {

enum MemProtFlag
{
	MPF_UNKNOWN		= 0,
	MPF_READ		= 1 << 0,
	MPF_WRITE		= 1 << 1,
	MPF_EXEC		= 1 << 2,
	MPF_GUARD		= 1 << 3,
	MPF_NO_ACCESS	= 1 << 4
};

void addLogMsg(const char* fmt, ...);
QString memoryProtectToStr(quint32 p);
MemProtFlag decodeMemoryProtect(quint32 p);
qlist<ea_t> codeRefsToCode(ea_t ea);
qlist<ea_t> dataRefsToCode(ea_t ea);
ea_t getNextCodeOrDataEA(ea_t ea, bool nonCodeNames);
bool isFuncStart(ea_t ea);
QMainWindow* findIDAMainWindow();

} // ida
} // util
