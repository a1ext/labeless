/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "util_ida.h"
#include "../labeless_ida.h"
#include "../compat.h"

// Qt
#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <QPointer>
#include <QString>
#include <QWidget>

// IDA
#include <expr.hpp>
#include <../ldr/idaldr.h>


namespace util {
namespace ida {


void addLogMsg(const char* fmt, ...)
{
	static const QString kRPCThread = "RPC Thread";
	QString message;
	do {
		va_list va;
		va_start(va, fmt);
		char buff[1024] = {};
		::qvsnprintf(buff, sizeof(buff), fmt, va);
		va_end(va);
		message = buff;
	} while (0);

	QMetaObject::invokeMethod(&Labeless::instance(), "onLogMessage", Qt::QueuedConnection,
		Q_ARG(QString, message),
		Q_ARG(QString, kRPCThread));
}

QString memoryProtectToStr(quint32 p)
{
	QString rv;

	const MemProtFlag flags = decodeMemoryProtect(p);

	if ((p & PAGE_READONLY) ||
		(p & PAGE_READWRITE) ||
		(p & PAGE_EXECUTE_READ) ||
		(p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY) ||
		(p & SEC_IMAGE))
		rv += "R";
	else
		rv += " ";

	if ((p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY) ||
		(p & PAGE_READWRITE) ||
		(p & PAGE_WRITECOMBINE))
		rv += "W";
	else
		rv += " ";

	if ((p & PAGE_EXECUTE) ||
		(p & PAGE_EXECUTE_READ) ||
		(p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY))
		rv += "E";
	else
		rv += " ";

	if (p & PAGE_NOACCESS)
		rv = "NO ACCESS";

	if (p & PAGE_GUARD)
		rv += " GUARD";
	return rv;
}

MemProtFlag decodeMemoryProtect(quint32 p)
{
	if (p & PAGE_NOACCESS)
		return MPF_NO_ACCESS;

	unsigned rv = MPF_UNKNOWN;
	if ((p & PAGE_READONLY) ||
		(p & PAGE_READWRITE) ||
		(p & PAGE_EXECUTE_READ) ||
		(p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY) ||
		(p & SEC_IMAGE))
	{
		rv |= MPF_READ;
	}

	if ((p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY) ||
		(p & PAGE_READWRITE) ||
		(p & PAGE_WRITECOMBINE))
	{
		rv |= MPF_WRITE;
	}

	if ((p & PAGE_EXECUTE) ||
		(p & PAGE_EXECUTE_READ) ||
		(p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY))
	{
		rv |= MPF_EXEC;
	}

	if (p & PAGE_GUARD)
		rv |= MPF_GUARD;

	return static_cast<MemProtFlag>(rv);
}

qlist<ea_t> codeRefsToCode(ea_t ea)
{
	if (!compat::is_code(compat::get_flags(ea)))
		return qlist<ea_t>();
	ea_t eCref = get_first_fcref_to(ea);
	if (eCref == BADADDR)
		return qlist<ea_t>();

	qlist<ea_t> refs;
	do {
		refs.push_back(eCref);
	} while ((eCref = get_next_fcref_to(ea, eCref)) != BADADDR);
	return refs;
}

qlist<ea_t> dataRefsToCode(ea_t ea)
{
	if (!compat::is_code(compat::get_flags(ea)))
		return qlist<ea_t>();
	ea_t eDref = get_first_dref_to(ea);
	if (eDref == BADADDR)
		return qlist<ea_t>();

	qlist<ea_t> refs;
	do {
		refs.push_back(eDref);
	} while ((eDref = get_next_dref_to(ea, eDref)) != BADADDR);
	return refs;
}

ea_t getNextCodeOrDataEA(ea_t ea, bool nonCodeNames)
{
	while (BADADDR != (ea = next_not_tail(ea)))
	{
		auto flags = get_aflags(ea);
		if ((flags & (1 << 0xE)))
			return ea;
		flags = compat::get_flags(ea);
		if (compat::is_enabled(ea) && (compat::is_code(flags) || (nonCodeNames && compat::is_data(flags))))
			return ea;
	}
	return BADADDR;
}

bool isFuncStart(ea_t ea)
{
	if (ea == BADADDR)
		return false;

	func_t* fn = get_func(ea);
	return fn && START_RANGE_EA(fn) == ea;
}

QMainWindow* findIDAMainWindow()
{
	static QPointer<QMainWindow> mainWindow;
	if (mainWindow)
		return mainWindow;

#if (IDA_SDK_VERSION < 700)
	if (WId hwnd = reinterpret_cast<WId>(callui(ui_get_hwnd).vptr))
	{
		if (mainWindow = qobject_cast<QMainWindow*>(QWidget::find(hwnd)))
			return mainWindow;
	}
#endif // IDA_SDK_VERSION < 700

	// fallback: when we cannot get the main window using callui
	static const QString kIDAMainWindowClassName = "IDAMainWindow";

	QWidgetList wl = qApp->allWidgets();
	for (int i = 0; i < wl.size(); ++i)
	{
		QString clsname = wl.at(i)->metaObject()->className();
		if (clsname == kIDAMainWindowClassName)
		{
			if (mainWindow = qobject_cast<QMainWindow*>(wl.at(i)))
				return mainWindow;
		}
	}

	return nullptr;
}

bool isExternSeg(::segment_t* s)
{
	if (!s)
		return false;

	if ((s->type & SEG_XTRN) != SEG_XTRN)
		return false;
	
#if (IDA_SDK_VERSION < 700)
	char segBuff[MAXSTR] = {};
	return get_segm_name(s, segBuff, MAXSTR) > 0 && std::string(segBuff) == NAME_EXTERN;
#else // IDA_SDK_VERSION < 700
	::qstring segname;
	return ::get_segm_name(&segname, s) > 0 && segname == NAME_EXTERN;
#endif // IDA_SDK_VERSION < 700	
}


} // ida
} // util