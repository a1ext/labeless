/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "hlp.h"
#include "labeless_ida.h"
#include "../common/version.h"
#include "jedi.h"

#if defined(__unix__) || defined(__linux__)
#include <string.h>
#endif

// IDA
#include <expr.hpp>

// std
#include <limits>
#include <sstream>

// Qt
#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <QPointer>
#include <QString>
#include <QWidget>


#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>

// kludge to avoid link with debug version of python27
#ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
#else
#  include <Python.h>
#endif // _DEBUG

#undef max // fix for std::numeric_limits<>::max()


namespace hlp
{

static const std::string base64_chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

static const std::string kExternKeyword = "__extern__";
static const std::string kResultKeyword = "__result__";
static const std::string kResultStrKeyword = "__result_str__";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	std::stringstream ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret << base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret << base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret << '=';

	}

	return ret.str();
}

std::string base64_decode(const std::string& encoded_string)
{
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::stringstream ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret << char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret << char_array_3[j];
	}

	return ret.str();
}

std::string unhexlify(const std::string& hexStr)
{
	const size_t cnt = hexStr.size() / 2;
	std::string rv(cnt, '\0');
	for (size_t i = 0; i < cnt; ++i)
		rv[i] = char(std::stoul(hexStr.substr(i * 2, 2), nullptr, 16));
	return rv;
}

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

std::string memoryProtectToStr(quint32 p)
{
	std::string rv;

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

qlist<ea_t> codeRefsToCode(ea_t ea)
{
	if (!isCode(get_flags_novalue(ea)))
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
	if (!isCode(get_flags_novalue(ea)))
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
		flags = get_flags_novalue(ea);
		if (::isEnabled(ea) && (isCode(flags) || (nonCodeNames && isData(flags))))
			return ea;
	}
	return BADADDR;
}

bool isFuncStart(ea_t ea)
{
	if (ea == BADADDR)
		return false;

	func_t* fn = get_func(ea);
	return fn && fn->startEA == ea;
}

QMainWindow* findIDAMainWindow()
{
	static QPointer<QMainWindow> mainWindow;
	if (mainWindow)
		return mainWindow;
	if (WId hwnd = reinterpret_cast<WId>(callui(ui_get_hwnd).vptr))
	{
		if (mainWindow = qobject_cast<QMainWindow*>(QWidget::find(hwnd)))
			return mainWindow;
	}

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

namespace idapython {

bool init()
{
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: python extlang not found\n", __FUNCTION__);
		return false;
	}
	char errbuff[MAXSTR] = {};
	static const std::string pyInitMsg = "import json;" + kResultKeyword + " = None\n"
		"idaapi.msg('Labeless: Python initialized... OK\\n')\n";
	if (!run_statements(pyInitMsg.c_str(), errbuff, _countof(errbuff), elng))
	{
		msg("%s: run_statements() failed\n", __FUNCTION__);
		if (::qstrlen(errbuff))
			msg("%s: error: %s", __FUNCTION__, errbuff);
		return false;
	}

	QString error;
	if (!jedi::init_completer(elng, error))
	{
		msg("Labeless: Unable to import `jedi` python module, auto-completion and intellisence won't be available in the python editors\n");
	}

	return true;
}

bool runScript(const std::string& script, std::string& externObj, std::string& error)
{
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}
	char errbuff[1024] = {};
	externObj.clear();
	error.clear();

	if (!run_statements(script.c_str(), errbuff, _countof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}

	idc_value_t rv;
	if (elng->calcexpr(BADADDR, ("json.dumps(" + kExternKeyword + ")").c_str(), &rv, errbuff, sizeof(errbuff)))
	{
		externObj = rv.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains("NameError"))
	{
		error = errbuff;
		return false;
	}
	return true;
}

bool setResultObject(const std::string& obj, std::string& error)
{
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}

	select_extlang(elng);

	char errbuff[1024] = {};
	error.clear();

	idc_value_t varStr(obj.c_str());

	if (!elng->set_attr(nullptr, kResultStrKeyword.c_str(), &varStr))
	{
		error = "unable to set " + kResultStrKeyword;
		return false;
	}

	const std::string stmt = kResultKeyword + " = json.loads(" + kResultStrKeyword + ")";
	if (!run_statements(stmt.c_str(), errbuff, _countof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}

	return true;
}

} // idapython

namespace python {

bool init(QString& error)
{
	Py_InteractiveFlag = 0; // ??

	Py_InitializeEx(0);

	if (!Py_IsInitialized())
	{
		error = "Cannot initialize Python";
		return false;
	}
	PyEval_InitThreads();

	// PyRun_SimpleString("import site");

	return true;
}

namespace {

DWORD pyExecExceptionFilter(DWORD code, _EXCEPTION_POINTERS* ep)
{
	// FIXME
	/*PyObject* msg = PyString_FromFormat("An exception occurred, code: 0x%x", code);
	stdErrHandler(nullptr, msg);	 
	Py_XDECREF(msg);*/
	return EXCEPTION_EXECUTE_HANDLER;
}

} // anonymous

bool safeRunString(const std::string& script, bool& exceptionOccured, std::string& error)
{
	bool rv = true;
	exceptionOccured = false;
	error.clear();

	__try
	{
		PyObject* m = PyImport_AddModule("__main__");
		if (!m)
			return false;
		PyObject* d = PyModule_GetDict(m);

		PyObject* v = PyRun_StringFlags(script.c_str(), Py_file_input, d, d, NULL);
		if (v == NULL)
		{
			PyErr_Print();
			return false;
		}

		rv &= true;


		Py_DECREF(v);
		if (Py_FlushLine())
			PyErr_Clear();
	}
	__except (pyExecExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
	{
		exceptionOccured = true;
		return false;
	}
	return rv;
}

} // python

namespace protobuf {

bool parseBigMessage(::google::protobuf::Message& msg, const std::string& data)
{
	static const int kProtobufMessageLimit = 0x40000000;

	::google::protobuf::io::CodedInputStream input(reinterpret_cast<const uint8*>(data.c_str()), data.size());
	input.SetTotalBytesLimit(kProtobufMessageLimit, kProtobufMessageLimit);
	return msg.ParseFromCodedStream(&input);
}

} // protobuf

namespace net {

qstring wsaErrorToString()
{
	qstring rv;
#ifdef __NT__
	const DWORD e = WSAGetLastError();
	rv.sprnt("\n0x%08X ", e);

	char* msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, e,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&msg, 0, nullptr);
	if (!msg)
		return rv;
	rv += msg;
	LocalFree((HLOCAL)msg);
#elif defined(__unix__) || defined(__linux__)
	if (auto errStr = strerror(errno))
		rv = errStr;
#endif
	return rv;
}

bool sockSendBuff(SOCKET s, const char* buff, uint64_t len)
{
	if (INVALID_SOCKET == s || !buff || !len)
		return false;

	static const int kMaxInt = std::numeric_limits<int>::max();
	uint64_t left = len;

	while (left > 0)
	{
		const int toSend = left > kMaxInt ? left % kMaxInt : static_cast<int>(left);
		const int retVal = send(s, buff, toSend, 0);
		if (retVal < 0)
		{
			addLogMsg(QString("send() failed. Error: %1\n").arg(wsaErrorToString().c_str()).toStdString().c_str());
			return false;
		}
		left -= retVal;
		buff += retVal;
	}

	return true;
}

bool sockSendString(SOCKET s, const std::string& str)
{
	if (INVALID_SOCKET == s || str.empty())
		return false;

	const char* ptrBuffer = str.c_str();
	static const int kMaxInt = std::numeric_limits<int>::max();

	uint64_t left = static_cast<uint64_t>(str.length());

	while (left > 0)
	{
		const int toSend = left > kMaxInt ? left % kMaxInt : static_cast<int>(left);

		const int retVal = send(s, ptrBuffer, toSend, 0);
		if (retVal < 0)
		{
			addLogMsg(QString("send() failed. Error: %1\n").arg(wsaErrorToString().c_str()).toStdString().c_str());
			return false;
		}
		left -= retVal;
		ptrBuffer += retVal;
	}
	return true;
}

bool sockRecvAll(SOCKET s, std::string& result)
{
	result.clear();

	static const uint kBuffSize = 0x5000;
	char buff[kBuffSize];
	int rv = 0;
	std::stringstream response;
	while ((rv = recv(s, buff, kBuffSize, 0)) > 0)
		response << std::string(buff, buff + rv);
	result = response.str();
	return true;
}

bool sendAll(SOCKET s, const std::string& buff, std::string& error)
{
	unsigned total_sent = 0;
	int sent;
	while (total_sent < buff.length())
	{
		if (SOCKET_ERROR == (sent = send(s, buff.c_str() + total_sent, buff.length() - total_sent, 0)))
		{
			error = wsaErrorToString().c_str();
			return false;
		}
		total_sent += static_cast<unsigned>(sent);
	}
	return total_sent == buff.size();
}

} // net

namespace github {

static const std::string kHost = "api.github.com";
static const std::string kPath = "/repos/a1ext/labeless/releases/latest";
static const std::string kPyCheckScript = "import json\r\n"
	"from httplib import HTTPSConnection\r\n"
	"c = HTTPSConnection('" + kHost + "')\r\n"
	"c.putrequest('GET', '" + kPath + "')\r\n"
	"c.putheader('User-Agent', '" LABELESS_PRODUCT "')\r\n"
	"c.putheader('Host', '" + kHost + "')\r\n"
	"c.endheaders()\r\n"
	"resp = c.getresponse()\r\n"
	"if resp.status != 200: raise Exception('Request failed (%d), reason: %s' % (resp.status, resp.reason))\r\n"
	"body = resp.read()\r\n"
	"del resp\r\n"
	"del c\r\n"
	"del HTTPSConnection\r\n"
	"j = json.loads(body)\r\n"
	"del body\r\n";

static const std::string kGetTagSTMT = "str(j['tag_name'])";
static const std::string kGetTagNameSTMT = "str(j['name'])";
static const std::string kGetUrlSTMT = "str(j['html_url'])";

bool getLatestRelease(ReleaseInfo& ri, std::string& error)
{
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}

	char errbuff[1024] = {};

	if (!run_statements(kPyCheckScript.c_str(), errbuff, sizeof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}

	idc_value_t val;
	if (elng->calcexpr(BADADDR, kGetTagSTMT.c_str(), &val, errbuff, sizeof(errbuff)))
	{
		ri.tag = val.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains("NameError"))
	{
		error = errbuff;
		return false;
	}

	if (elng->calcexpr(BADADDR, kGetTagNameSTMT.c_str(), &val, errbuff, sizeof(errbuff)))
	{
		ri.name = val.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains("NameError"))
	{
		error = errbuff;
		return false;
	}

	if (elng->calcexpr(BADADDR, kGetUrlSTMT.c_str(), &val, errbuff, sizeof(errbuff)))
	{
		ri.url = val.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains("NameError"))
	{
		error = errbuff;
		return false;
	}
	return true;
}

} // github

} // hlp

