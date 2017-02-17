/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include <string>
#include <QStringList>
#include "../jedi.h"

namespace util {

namespace python {

// direct Python bindings
bool init(QString& error);
bool safeRunString(const std::string& script, bool& exceptionOccured, std::string& error);

namespace jedi {

bool init(QString& error);
bool is_available();
bool get_completions(const QString& script,
	int zline,
	int zcol,
	QStringList& completions,
	::jedi::SignatureMatchList& signatureMatches,
	QString& error);

} // jedi
} // python
} // util
