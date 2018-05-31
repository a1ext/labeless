/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "labeless.h"

#include <fstream>
#include <intsafe.h>
#include <inttypes.h>
#include <memory>
#include <mstcpip.h>
#include <regex>
#include <tchar.h>
#include <time.h>

#include <strsafe.h>

#include "util.h"
#include "labeless_x64dbg.h"

#include "../common/cpp/rpc.pb.h"
#include "../common/version.h"

#include "pluginsdk/_scriptapi_gui.h"


extern "C" {
#ifdef _WIN64
	void init_x64dbgapi64();
#else // _WIN64
	void init_x64dbgapi();
#endif // _WIN64
};

#ifdef ENABLE_PYTHON_PROFILING
#include <compile.h>
#include <frameobject.h>
#endif

namespace {

static struct StaticConfig
{
	HWND		helperWnd = nullptr;
	UINT		hlpLogMessageId = 0;
	UINT		hlpCommandReceived = 0;
	UINT		hlpPortChanged = 0;
} gConfig;

static const char kBackendName[] {"x64dbg"};


#ifdef ENABLE_PYTHON_PROFILING
int tracefunc(PyObject *obj, _frame *frame, int what, PyObject *arg)
{
	if (what != PyTrace_LINE)
		return 0;

	if (PyObject* str = PyObject_Str(frame->f_code->co_filename))
	{
		const std::string s = PyString_AsString(str);
		log_r("PROFILING: %s:%d", s.c_str(), frame->f_lineno);
		Py_DECREF(str);
		std::ofstream of("c:\\labeless_trace.log", std::ios_base::app);
		if (of)
		{
			char buff[128] = {};
			_strdate_s(buff, _countof(buff));
			of << std::string(buff);
			_strtime_s(buff, _countof(buff));
			of << " " << std::string(buff) << ": " << s << " " << frame->f_lineno << std::endl;
			of.close();
		}
	}
	return 0;
}
#endif // ENABLE_PYTHON_PROFILING

bool execFile(const xstring& fileName)
{
#ifdef _UNICODE
	PyObject* const pyStrName = PyUnicode_FromWideChar(fileName.c_str(), fileName.length());
	if (!pyStrName)
		return false;

	PyObject* const pyUtf8Name = PyUnicode_AsMBCSString(pyStrName);
	Py_XDECREF(pyStrName);
	char* zUtf8name = PyString_AsString(pyUtf8Name);
	const auto size = PyString_GET_SIZE(zUtf8name);
	PyObject* const pyFileObject = PyFile_FromString(zUtf8name, "r");

	const bool rv = PyRun_SimpleFile(PyFile_AsFile(pyFileObject), zUtf8name) == 0;
	Py_XDECREF(pyUtf8Name);
#else
	PyObject* const pyFileObject = PyFile_FromString(const_cast<char*>(fileName.c_str()), "r");

	if (!pyFileObject)
		return false;

	const bool rv = PyRun_SimpleFile(PyFile_AsFile(pyFileObject), const_cast<char*>(fileName.c_str())) == 0;
#endif

	Py_XDECREF(pyFileObject);
	return rv;
}

/* Obtains a string from a Python traceback.
This is the exact same string as "traceback.print_exc" would return.

Pass in a Python traceback object (probably obtained from PyErr_Fetch())
*/

bool pyTraceback_AsString(PyObject* exc_tb, std::string& result)
{
#define TRACEBACK_FETCH_ERROR(what) { errMsg = what; break; }

	result.clear();
	char* errMsg = NULL; /* holds a local error message */
	PyObject* modStringIO = NULL;
	PyObject* modTB = NULL;
	PyObject* obFuncStringIO = NULL;
	PyObject* obStringIO = NULL;
	PyObject* obFuncTB = NULL;
	PyObject* argsTB = NULL;
	PyObject* obResult = NULL;

	do {
		/* Import the modules we need - cStringIO and traceback */
		modStringIO = PyImport_ImportModule("cStringIO");
		if (modStringIO == NULL)
			TRACEBACK_FETCH_ERROR("cant import cStringIO\n");

		modTB = PyImport_ImportModule("traceback");
		if (modTB == NULL)
			TRACEBACK_FETCH_ERROR("cant import traceback\n");
		/* Construct a cStringIO object */
		obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
		if (obFuncStringIO == NULL)
			TRACEBACK_FETCH_ERROR("cant find cStringIO.StringIO\n");
		obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
		if (obStringIO == NULL)
			TRACEBACK_FETCH_ERROR("cStringIO.StringIO() failed\n");
		/* Get the traceback.print_exception function, and call it. */
		obFuncTB = PyObject_GetAttrString(modTB, "print_tb");
		if (obFuncTB == NULL)
			TRACEBACK_FETCH_ERROR("cant find traceback.print_tb\n");

		argsTB = Py_BuildValue("OOO", exc_tb ? exc_tb : Py_None, Py_None, obStringIO);
		if (argsTB == NULL)
			TRACEBACK_FETCH_ERROR("cant make print_tb arguments\n");

		obResult = PyObject_CallObject(obFuncTB, argsTB);
		if (obResult == NULL)
			TRACEBACK_FETCH_ERROR("traceback.print_tb() failed\n");
		/* Now call the getvalue() method in the StringIO instance */
		Py_DECREF(obFuncStringIO);
		obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
		if (obFuncStringIO == NULL)
			TRACEBACK_FETCH_ERROR("cant find getvalue function\n");
		Py_DECREF(obResult);
		obResult = PyObject_CallObject(obFuncStringIO, NULL);
		if (obResult == NULL)
			TRACEBACK_FETCH_ERROR("getvalue() failed.\n");

		/* And it should be a string all ready to go - duplicate it. */
		if (!PyString_Check(obResult))
			TRACEBACK_FETCH_ERROR("getvalue() did not return a string\n");

		char* tempResult = PyString_AsString(obResult);
		if (tempResult)
			result = tempResult;
	} while (0);

	/* All finished - first see if we encountered an error */
	if (result.empty() && errMsg != NULL) {
		/*const size_t len = strlen(errMsg) + 1;
		result = reinterpret_cast<char*>(PyMem_Malloc(len));
		if (result != NULL)
			// if it does, not much we can do! 
			strcpy_s(result, len, errMsg);*/
		result = errMsg;
	}
	Py_XDECREF(modStringIO);
	Py_XDECREF(modTB);
	Py_XDECREF(obFuncStringIO);
	Py_XDECREF(obStringIO);
	Py_XDECREF(obFuncTB);
	Py_XDECREF(argsTB);
	Py_XDECREF(obResult);
	return true;
}

#ifdef _UNICODE
#	define VSNXprintf_s _vsnwprintf_s
#	define Xncpy_s wcsncpy_s
#	define Xlen wcslen
#else // _UNICODE
#	define VSNXprintf_s vsnprintf_s
#	define Xncpy_s strncpy_s
#	define Xlen strlen
#endif // _UNICODE

inline void server_log(const TCHAR* fmt, ...)
{
	if (!IsWindow(gConfig.helperWnd))
		return;
	std::vector<TCHAR> buff(MAX_STRING_SIZE, '\0');
	va_list v;
	va_start(v, fmt);
	VSNXprintf_s(&buff[0], buff.size(), _TRUNCATE, fmt, v);
	va_end(v);
	const auto len = Xlen(&buff[0]);
	TCHAR* msg = new TCHAR[len + 1];
	Xncpy_s(msg, len + 1, &buff[0], MAX_STRING_SIZE);
	PostMessage(gConfig.helperWnd, gConfig.hlpLogMessageId, reinterpret_cast<WPARAM>(msg), static_cast<LPARAM>(len));
}

static PyObject* stdOutHandler(PyObject* self, PyObject* arg)
{
	const char* str = PyString_AsString(arg);

	auto& cd = Labeless::instance().clientData();
	recursive_lock_guard lock(cd.stdOutLock);
	cd.stdOut << str << "\n";

	Py_RETURN_NONE;
}

static PyObject* stdErrHandler(PyObject*, PyObject* arg)
{
	const char* str = PyString_AsString(arg);
	auto& cd = Labeless::instance().clientData();
	recursive_lock_guard lock(cd.stdErrLock);
	cd.stdErr << str << "\n";

	Py_RETURN_NONE;
}

static PyObject* setBinaryResult(PyObject*, PyObject* arg)
{
	if (!PyTuple_Check(arg))
		Py_RETURN_NONE;

	PyObject* pyJobId = nullptr;
	PyObject* pyBuff = nullptr;
	if (!PyArg_UnpackTuple(arg, "setBinaryResult", 2, 2, &pyJobId, &pyBuff))
	{
		if (PyErr_Occurred())
			PyErr_Print();
		Py_RETURN_NONE;
	}

	uint64_t jobId = 0;
	if (PyInt_Check(pyJobId))
	{
		jobId = static_cast<uint64_t>(PyInt_AsLong(pyJobId));
	}
	else if (PyLong_Check(pyJobId))
	{
		jobId = PyLong_AsUnsignedLongLong(pyJobId);
	}
	else
	{
		if (PyErr_Occurred())
			PyErr_Print();
		log_r("Invalid jobId type, should be int or long");
		Py_RETURN_NONE;
	}

	if (!PyObject_CheckBuffer(pyBuff))
	{
		Py_RETURN_NONE;
	}

	Py_ssize_t size = 0;
	const char* buff = nullptr;

	if (PyObject_AsCharBuffer(pyBuff, &buff, &size) >= 0 && buff)
	{
		auto& cd = Labeless::instance().clientData();
		recursive_lock_guard lock(cd.commandsLock);
		Request* r = cd.find(jobId);
		if (r)
			r->binaryResult = std::string(buff, size);
		else
			log_r("Unable to set binary result, no commands found for jobId: %llu", jobId);
	}
	Py_RETURN_NONE;
}

static PyObject* get_params(PyObject*, PyObject* arg)
{
	uint64_t jobId = 0;
	if (PyInt_Check(arg))
	{
		jobId = static_cast<uint64_t>(PyInt_AsLong(arg));
	}
	else if (PyLong_Check(arg))
	{
		jobId = PyLong_AsUnsignedLongLong(arg);
	}
	else
	{
		if (PyErr_Occurred())
			PyErr_Print();
		log_r("Invalid jobId type, should be int or long");
		Py_RETURN_NONE;
	}
	if (!jobId)
		Py_RETURN_NONE;

	ClientData& cd = Labeless::instance().clientData();
	recursive_lock_guard lock(cd.commandsLock);
	std::string params;
	for (const Request& r : cd.commands)
	{
		if (r.id == jobId)
			return PyString_FromStringAndSize(r.params.c_str(), r.params.size());;
	}

	Py_RETURN_NONE;
}

static PyObject* olly_log(PyObject*, PyObject* arg)
{
	if (PyString_Check(arg))
		server_log(_T("%s"), util::to_xstr(PyString_AsString(arg)).c_str());
	Py_RETURN_NONE;
}

static PyObject* olly_set_error(PyObject*, PyObject* arg)
{
	if (!PyTuple_Check(arg))
		Py_RETURN_NONE;

	PyObject* pyJobId = nullptr;
	PyObject* pyErrorStr = nullptr;
	if (!PyArg_UnpackTuple(arg, "set_error", 2, 2, &pyJobId, &pyErrorStr))
	{
		if (PyErr_Occurred())
			PyErr_Print();
		Py_RETURN_NONE;
	}

	uint64_t jobId = 0;
	if (PyInt_Check(pyJobId))
	{
		jobId = static_cast<uint64_t>(PyInt_AsLong(pyJobId));
	}
	else if (PyLong_Check(pyJobId))
	{
		jobId = PyLong_AsUnsignedLongLong(pyJobId);
	}
	else
	{
		if (PyErr_Occurred())
			PyErr_Print();
		log_r("Invalid jobId type, should be int or long");
		Py_RETURN_NONE;
	}

	if (!PyString_Check(pyErrorStr))
		Py_RETURN_NONE;

	Py_ssize_t size = 0;
	const char* buff = nullptr;

	auto errorStr = PyString_AsString(pyErrorStr);
	if (!errorStr)
		Py_RETURN_NONE;

	auto& cd = Labeless::instance().clientData();
	recursive_lock_guard lock(cd.commandsLock);
	Request* r = cd.find(jobId);
	if (r)
		r->error = errorStr;
	else
		log_r("Unable to set error string, no commands found");
	Py_RETURN_NONE;
}

static PyObject* olly_get_ver(PyObject*, PyObject* arg)
{
	std::string wver = LABELESS_VER_STR;
	PyObject* tmp = PyString_FromStringAndSize(reinterpret_cast<const char*>(wver.c_str()), wver.length());
	if (!tmp)
	{
		PyErr_Print();
		Py_RETURN_NONE;
	}
	PyObject* rv = PyString_AsDecodedString(tmp, "windows_1251", "replace");
	Py_XDECREF(tmp);
	return rv;
}

static PyObject* olly_get_hprocess(PyObject*, PyObject*)
{
	if (g_pi.hProcess)
		return PyInt_FromSize_t(reinterpret_cast<size_t>(g_pi.hProcess));
	Py_RETURN_NONE;
}

static PyObject* olly_get_pid(PyObject*, PyObject*)
{
	if (g_pi.dwProcessId)
		return PyInt_FromLong(g_pi.dwProcessId);
	Py_RETURN_NONE;
}

static PyObject* olly_get_backend_name(PyObject*, PyObject*)
{
	return PyString_FromString(kBackendName);
}

static PyObject* olly_get_backend_info(PyObject*, PyObject*)
{
	PyObject* const rv = PyDict_New();

#ifdef _WIN64
	static const char kBitness [] = "64";
#else // _WIN64
	static const char kBitness [] = "32";
#endif // _WIN64
	PyDict_SetItemString(rv, "bitness", PyString_FromString(kBitness));
	PyDict_SetItemString(rv, "name", PyString_FromString(kBackendName));
	return rv;
}

// Register the wrapped functions.
static PyMethodDef PyOllyMethods [] =
{
	{ "std_out_handler", stdOutHandler, METH_O, "stdout handler" },
	{ "std_err_handler", stdErrHandler, METH_O, "stderr handler" },
	{ "set_binary_result", setBinaryResult, METH_VARARGS, "binary result handler" },
	{ "get_params", get_params, METH_O, "get RPC call parameters" },
	{ "olly_log", olly_log, METH_O, "Olly log output" },
	{ "set_error", olly_set_error, METH_VARARGS, NULL },
	{ "labeless_ver", olly_get_ver, METH_NOARGS, "get Labeless version" },
	{ "get_hprocess", olly_get_hprocess, METH_NOARGS, "get hProcess of debuggee" },
	{ "get_pid", olly_get_pid, METH_NOARGS, "get PID of debuggee" },
	{ "get_backend_info", olly_get_backend_info, METH_NOARGS, "get backend info\n:return: {'name': '', 'bitness' : ''}" },
	{ NULL, NULL, 0, NULL }
};

static const std::string kExternKeyword = "__extern__";
static const std::string kResultKeyword = "__result__";
static const std::string kJsonModuleName = "json";
static const std::string kJsonLoadsFuncName = "loads";
static const std::string kLabelessPythonModuleName = "labeless";
static const std::string kLabelessSerializeResultFuncName = "serialize_result";

DWORD pyExecExceptionFilter(DWORD code, _EXCEPTION_POINTERS* ep)
{
	PyObject* msg = PyString_FromFormat("An exception occurred, code: 0x%x", code);
	stdErrHandler(nullptr, msg);
	Py_XDECREF(msg);
	return EXCEPTION_EXECUTE_HANDLER;
}

PyObject* pyDeserializeObjectFromJsonString(const std::string& jsonStr)
{
	PyObject* pyJson = nullptr;
	PyObject* pyJsonLoads = nullptr;
	PyObject* pyJsonStr = nullptr;
	PyObject* pyArgs = nullptr;
	PyObject* result = nullptr;

	if ((pyJson = PyImport_ImportModule(kJsonModuleName.c_str())) &&
		(pyJsonLoads = PyObject_GetAttrString(pyJson, kJsonLoadsFuncName.c_str())) &&
		(pyJsonStr = PyString_FromString(jsonStr.c_str())) &&
		(pyArgs = PyTuple_Pack(1, pyJsonStr)))
	{
		result = PyObject_Call(pyJsonLoads, pyArgs, NULL);
	}

	Py_XDECREF(pyArgs);
	Py_XDECREF(pyJsonStr);
	Py_XDECREF(pyJsonLoads);
	Py_XDECREF(pyJson);

	return result;
}

bool pySerializeObjectToJson(PyObject* o, std::string& rv)
{
	PyObject* pyModuleDict = nullptr;
	PyObject* pyLLstr = nullptr;
	PyObject* pyLL = nullptr;
	PyObject* pySerializeResultFn = nullptr;
	PyObject* pyArgs = nullptr;
	PyObject* result = nullptr;

	rv.clear();
	bool isOk = false;

	if ((pyModuleDict = PyImport_GetModuleDict()) &&
		(pyLLstr = PyString_FromString(kLabelessPythonModuleName.c_str())) &&
		(PyDict_Contains(pyModuleDict, pyLLstr) == 1) &&
		(pyLL = PyDict_GetItem(pyModuleDict, pyLLstr)) &&
		(pySerializeResultFn = PyObject_GetAttrString(pyLL, kLabelessSerializeResultFuncName.c_str())) &&
		(pyArgs = PyTuple_Pack(1, o)) &&
		(result = PyObject_Call(pySerializeResultFn, pyArgs, NULL)) &&
		PyString_Check(result))
	{
		rv = PyString_AsString(result);
		isOk = true;
	}

	Py_XDECREF(result);
	Py_XDECREF(pyArgs);
	Py_XDECREF(pySerializeResultFn);
	Py_XDECREF(pyLLstr);

	return isOk;
}

static bool safePyRunSimpleString(const std::string& script, const std::string& scriptExternObj, bool& exceptionOccured, std::string& resultObj)
{
	bool rv = true;
	exceptionOccured = false;
	resultObj.clear();

	__try
	{
		PyObject* m = PyImport_AddModule("__main__");
		if (!m)
			return false;
		PyObject* d = PyModule_GetDict(m);

		if (!scriptExternObj.empty())
		{
			PyObject* pyExternObj = pyDeserializeObjectFromJsonString(scriptExternObj);

			if (pyExternObj)
				PyDict_SetItemString(d, kExternKeyword.c_str(), pyExternObj);
			else
				PyErr_PrintEx(0);

			rv = !!pyExternObj;
			Py_XDECREF(pyExternObj);
		}
		else
		{
			PyObject* pyExternKey = PyString_FromString(kExternKeyword.c_str());
			if (PyDict_Contains(d, pyExternKey) == 1)
			{
				PyDict_DelItemString(d, kExternKeyword.c_str());
			}
			Py_XDECREF(pyExternKey);
		}
		PyObject* v = PyRun_StringFlags(script.c_str(), Py_file_input, d, d, NULL);
		if (v == NULL)
		{
			PyErr_Print();
			return false;
		}

		rv &= true;

		// retrieve __result__ object and try to serialize it...
		PyObject* pyResultKey = PyString_FromString(kResultKeyword.c_str());
		if (PyDict_Contains(d, pyResultKey) == 1)
		{
			if (PyObject* pyResultObj = PyDict_GetItem(d, pyResultKey))
			{
				pySerializeObjectToJson(pyResultObj, resultObj);
			}
		}
		Py_XDECREF(pyResultKey);

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

xstring wsaErrorToString()
{
	xstringstream rv;
	const DWORD e = WSAGetLastError();
	rv << std::hex << std::showbase << e << _T(" ");

	TCHAR* msg = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, e,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (TCHAR*)&msg, 0, nullptr);
	if (!msg)
		return rv.str();
	rv << msg;
	LocalFree((HLOCAL)msg);
	return rv.str();
}

bool sendAll(SOCKET s, const std::string& buff, xstring& error)
{
	size_t total_sent = 0;
	int sent;
	while (total_sent < buff.length())
	{
		if (SOCKET_ERROR == (sent = send(s, buff.c_str() + total_sent, static_cast<int>(buff.length() - total_sent), 0)))
		{
			error = wsaErrorToString();
			return false;
		}
		total_sent += static_cast<size_t>(sent);
	}
	const auto rv = total_sent == buff.size();
	if (!rv)
		error = _T("Sent bytes number differs from buffer length");
	return rv;
}

void protobufLogHandler(::google::protobuf::LogLevel level, const char* filename, int line, const std::string& message)
{
	if (level < ::google::protobuf::LOGLEVEL_WARNING)
		return;

	std::string levelStr;
	switch (level)
	{
	case google::protobuf::LOGLEVEL_INFO:
		levelStr = "[INFO] ";
		break;
	case google::protobuf::LOGLEVEL_WARNING:
		levelStr = "[WARN] ";
		break;
	case google::protobuf::LOGLEVEL_ERROR:
		levelStr = "[ERRO] ";
		break;
	case google::protobuf::LOGLEVEL_FATAL:
		levelStr = "[FATA] ";
		break;
	default:
		break;
	}
	server_log(_T("protobuf: %s %s:%u %s"), levelStr.c_str(), filename, line, message.c_str());
}


} // anonymous

void storePort(WORD port);
void storeFilterIP(const xstring& filter);

Request* ClientData::find(uint64_t jobId)
{
	for (size_t i = 0, e = commands.size(); i < e; ++i)
		if (commands[i].id == jobId)
			return &commands[i];
	return nullptr;
}

bool ClientData::remove(uint64_t jobId)
{
	for (auto it = commands.cbegin(), end = commands.cend(); it != end; ++it)
	{
		if (it->id == jobId)
		{
			commands.erase(it);
			return true;
		}
	}
	return false;
}

std::atomic_bool Labeless::m_ServerEnabled{ false };

Labeless::Labeless()
	: m_hInst(nullptr)
	, m_Port(defaultPort())
	, m_LogList(nullptr)
{
	//__asm __volatile finit; // Stupid Olly's bug fix (TODO: check is x64dbg works without this)
	::google::protobuf::SetLogHandler(protobufLogHandler);
}

Labeless::~Labeless()
{
	destroy();
}

Labeless& Labeless::instance()
{
	static Labeless ll;
	return ll;
}

bool Labeless::init(PLUG_SETUPSTRUCT*)
{
	if (!createWindow())
	{
		log_r("createWindow() failed.");
		return false;
	}
	if (!startServer())
	{
		log_r("startServer() failed.");
		return false;
	}
	return true;
}

bool Labeless::destroy()
{
	static bool destroyed = false;
	if (destroyed)
		return true;
	destroyed = true;

	if (gConfig.helperWnd)
		DestroyWindow(gConfig.helperWnd);
	gConfig.helperWnd = nullptr;
	stopServer();
	destroyPython();
	google::protobuf::ShutdownProtobufLibrary();
	return true;
}

bool Labeless::initPython()
{
	xstring pythonDir = util::getHostAppDir();
#ifdef ENABLE_PYTHON_ZIP
	const std::string zipPath = pythonDir + "\\python27.zip";

	SetEnvironmentVariable("PYTHONPATH", zipPath.c_str());

	Py_NoSiteFlag = 1;
	Py_SetPythonHome(const_cast<char*>(pythonDir.c_str()));
#endif // ENABLE_PYTHON_ZIP
	Py_InteractiveFlag = 0;

	Py_SetProgramName("");
	Py_InitializeEx(0);

#ifdef ENABLE_PYTHON_PROFILING
	//PyEval_SetTrace(tracefunc, NULL);
#endif

	if (!Py_IsInitialized())
	{
		log_r("Could not initialize Python");
		return false;
	}
	PyEval_InitThreads();

#ifdef ENABLE_PYTHON_ZIP
	PyRun_SimpleString("import sys\nsys.path.extend(['.', 'python_dlls', 'python27.zip', 'python27.zip/site-packages'])");
#endif // ENABLE_PYTHON_ZIP

	Py_InitModule("_py_olly", PyOllyMethods);
	const auto importSiteOK = PyRun_SimpleString("import site") == 0;
	if (!importSiteOK)
	{
		log_r("\"import site\" failed\n");
	}

#ifdef _WIN64
	init_x64dbgapi64();
#else // _WIN64
	init_x64dbgapi();
#endif // _WIN64

	pythonDir += _T("\\labeless_scripts");
	const xstring addLoacalFolderToPath = _T("import sys\nsys.path.extend([r\"\"\"") + pythonDir + _T("\"\"\"])");

	PyRun_SimpleString(addLoacalFolderToPath.c_str());
	const auto labelessOk = PyRun_SimpleString("import labeless as ll") == 0;
	if (!labelessOk)
	{
		logInitPythonFail("\"import labeless as ll\" failed.");
		return false;
	}

	const auto pyexcoreOk = PyRun_SimpleString("from labeless import pyexcore") == 0;
	if (!pyexcoreOk)
	{
		logInitPythonFail("\"from labeless import pyexcore\" failed.");
		return false;
	}

	return true;
}

void Labeless::logInitPythonFail(const std::string& info) const
{
	log_r("%s", info.c_str());

	std::string error;
	if (PyErr_Occurred())
	{
		PyObject* ptTB = nullptr;
		PyErr_Fetch(nullptr, nullptr, &ptTB);
		pyTraceback_AsString(ptTB, error);
	}
	const std::string& sErr = clientData().stdErr.str();
	const std::string& sOut = clientData().stdOut.str();
	if (!sErr.empty() || !sOut.empty())
	{
		if (!sErr.empty())
		{
			std::deque<std::string> lines = util::split(sErr);
			for (auto v : lines)
			{
				log_r_no_fn("%s", v.c_str());
			}
		}

#ifdef LABELESS_ADDITIONAL_LOGGING
		std::ofstream of("c:\\labeless.log", std::ios_base::app);
		if (of)
		{
			char buff[128] = {};
			_strdate_s(buff, 128);
			of << "\r\n" << std::string(buff);
			_strtime_s(buff, 128);
			of << " " << std::string(buff) << " FAILED TO INIT PYTHON, STDERR: " << sErr << std::endl
				<< "STDOUT:" << sOut;
			if (!error.empty())
				of << "\nTraceBack:\n" << error;
			of.close();
		}
#endif // LABELESS_ADDITIONAL_LOGGING
	}
}

void Labeless::destroyPython()
{
	Py_Finalize();
}

HWND Labeless::createWindow()
{
	static xstring kLabelessHelperWndClassName = util::randStr(32);
	if (gConfig.helperWnd)
		return gConfig.helperWnd;

	if (!gConfig.hlpLogMessageId && !(gConfig.hlpLogMessageId = RegisterWindowMessage(_T("{B221E840-FBD2-4ED3-A69E-3DDAB1F7EC36}"))))
	{
		log_r("RegisterWindowMessage(hlpLogMessageId) failed. LastError: %08X", GetLastError());
		return false;
	}
	if (!gConfig.hlpCommandReceived && !(gConfig.hlpCommandReceived = RegisterWindowMessage(_T("{79F0D105-76FF-40DB-9448-E9D9E5BA7938}"))))
	{
		log_r("RegisterWindowMessage(hlpCommandReceived) failed. LastError: %08X", GetLastError());
		return false;
	}
	if (!gConfig.hlpPortChanged && !(gConfig.hlpPortChanged = RegisterWindowMessage(_T("{774A37C9-6398-44AD-8F07-A421B55F0435}"))))
	{
		log_r("RegisterWindowMessage(hlpPortChanged) failed. LastError: %08X", GetLastError());
		return false;
	}

	HWND rv = nullptr;
	WNDCLASS wClass = {};

	wClass.lpfnWndProc = helperWinProc;
	wClass.hInstance = m_hInst;
	wClass.lpszClassName = kLabelessHelperWndClassName.c_str();
	wClass.hbrBackground = (HBRUSH) (16);

	if (RegisterClass(&wClass) == 0)
	{
		log_r("RegisterClass() failed.");
		return nullptr;
	}
	RECT rect = {};
	GetWindowRect(g_hwndDlg, &rect);

	rv = CreateWindow(kLabelessHelperWndClassName.c_str(), "", WS_CHILDWINDOW /*| WS_VISIBLE| WS_CAPTION*/,
		0, 0, 100, 100, g_hwndDlg, NULL, m_hInst, NULL);
	if (!rv)
	{
		log_r("CreateWindow() failed.");
		return rv;
	}
	gConfig.helperWnd = rv;
	return rv;
}

void Labeless::onSetPortRequested()
{
	duint port = m_Port;
	if (!Script::Gui::InputValue("Enter port value", &port) || port == m_Port)
		return;
	if (!port && port != m_Port)
	{
		port = defaultPort();
		log_r("Fallig back to default port: %u.", port);
	}
	stopServer();
	m_Port = static_cast<WORD>(port);
	startServer();
	storePort(m_Port);
}

void Labeless::onSetIPFilter()
{
	char buff[GUI_MAX_LINE_SIZE] = {};
	if (!Script::Gui::InputLine("Enter IP in format XXX.XXX.XXX.XXX", buff))
		return;
	CharLowerA(buff);

	xstring s = buff;
	const std::regex re(_T("^(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.")
		_T("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.")
		_T("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.")
		_T("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])$"));
	std::smatch m;
	if (s.empty() || s == _T("any") || s == _T("0.0.0.0"))
	{
		m_FilterIP.clear();
		storeFilterIP(_T(""));
		return;
	}
	if (!std::regex_search(s, m, re))
		return;
	m_FilterIP = m[1].str() + _T(".") + m[2].str() + _T(".") + m[3].str() + _T(".") + m[4].str();
	storeFilterIP(m_FilterIP);
}

void Labeless::stopServer()
{
	m_ServerEnabled = false;

	recursive_lock_guard lock(m_ThreadLock);
	if (!m_Thread)
		return;
	if (m_Thread->joinable())
		m_Thread->join();
	m_Thread.reset();
}

bool Labeless::startServer()
{
	if (m_ServerEnabled)
		return false;
	m_ServerEnabled = true;

	recursive_lock_guard lock(m_ThreadLock);
	if (m_Thread)
		return true;
	m_Thread.reset(new std::thread(Labeless::serverThread, this));

	log_r("Server thread started.");
	return true;
}

bool Labeless::bindAndListenSock(SOCKET& rv, WORD& wPort, const std::string& ip)
{
	rv = INVALID_SOCKET;
	if ((rv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		server_log(_T("socket() failed. Error %08X"), WSAGetLastError());
		return false;
	}
	BOOL bTrue = TRUE;
	if (SOCKET_ERROR == setsockopt(rv, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&bTrue), sizeof(bTrue)))
	{
		server_log(_T("setsockopt(SO_KEEPALIVE) failed. Error %08X"), WSAGetLastError());
		return false;
	}
	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	if (ip.empty())
		sin.sin_addr.s_addr = INADDR_ANY;
	else
		sin.sin_addr.s_addr = inet_addr(ip.c_str());
	server_log(_T("trying to bind()..."));
	for (; wPort < USHORT_MAX; ++wPort)
	{
		sin.sin_port = htons(wPort);
		if (SOCKET_ERROR != bind(rv, reinterpret_cast<SOCKADDR*>(&sin), sizeof(sin)))
			break;
	}
	const auto& bindAddr = util::inetAddrToString(&sin);
	server_log(_T("ok, binded at %s"), bindAddr.c_str());
	if (SOCKET_ERROR == listen(rv, SOMAXCONN))
	{
		server_log(_T("listen() failed. Error %08X"), WSAGetLastError());
		return false;
	}
	return true;
}

void WINAPI Labeless::serverThread(Labeless* ll)
{
	static bool wsaInitialized = false;
	if (!wsaInitialized)
	{
		WSADATA wd = {};
		if (WSAStartup(MAKEWORD(2, 2), &wd))
		{
			server_log(_T("WSAStartup() failed"));
			return;
		}
		wsaInitialized = true;
	}

	ClientData& client = ll->clientData();

	SOCKET sockets[2] { INVALID_SOCKET, INVALID_SOCKET };
	WORD wPort = ll->m_Port;

	if (!bindAndListenSock(sockets[0], wPort))
	{
		server_log(_T("Unable to start server"));
		return;
	}

	if (ll->m_Port != wPort)
	{
		server_log(_T("Serving port differs from specified in configuration. %u"), wPort);
		ll->m_Port = wPort;
		PostMessage(gConfig.helperWnd, gConfig.hlpPortChanged, 0, 0);
	}

	WSAEVENT evts[2] = {};
	evts[0] = WSACreateEvent();
	WSAEventSelect(sockets[0], evts[0], FD_ACCEPT | FD_CLOSE);
	DWORD actualClients = 1;
	while (Labeless::m_ServerEnabled)
	{
		DWORD index = WSAWaitForMultipleEvents(actualClients, evts, FALSE, 1000, FALSE);
		index -= WSA_WAIT_EVENT_0;
		if (WSA_WAIT_FAILED == index || WSA_WAIT_TIMEOUT == index)
		{
			// look for command result
			if (client.s != INVALID_SOCKET)
			{
				recursive_lock_guard lock(client.commandsLock);
				if (!client.commands.empty() && client.commands.back().finished)
				{
					xstring sendError;
					if (!sendAll(client.s, client.commands.back().result, sendError))
						server_log(_T("%s: send() failed, error: %s"), util::to_xstr(__FUNCTION__).c_str(), sendError.c_str());
					closesocket(client.s);

					server_log(_T("%s: jobId %llu socket %08X Response sent, len: 0x%08X"), util::to_xstr(__FUNCTION__).c_str(),
						client.commands.back().id, unsigned(client.s), client.commands.back().result.length());
					client.s = INVALID_SOCKET;
					if (!client.commands.back().background)
					{
						server_log(_T("%s: jobId %llu is removed"), util::to_xstr(__FUNCTION__).c_str(), client.commands.back().id);
						client.commands.pop_back();
					}
					else
					{
						client.commands.back().finished = false;
						client.commands.back().result.clear();
					}

					actualClients = 1;
					WSACloseEvent(evts[1]);
					evts[1] = 0;
				}
			}
			continue;
		}
		WSANETWORKEVENTS e = {};
		if (SOCKET_ERROR == WSAEnumNetworkEvents(sockets[index], evts[index], &e))
			continue;
		if ((e.lNetworkEvents & FD_ACCEPT) && e.iErrorCode[FD_ACCEPT_BIT] == 0)
		{
			if (!ll->onClientSockAccept(sockets[index], client))
				server_log(_T("onClientSockAccept() failed."));
			else
			{
				evts[1] = WSACreateEvent();
				if (actualClients == 1)
					actualClients = 2;
				sockets[1] = client.s;
				WSAEventSelect(client.s, evts[1], FD_READ | FD_CLOSE);
				server_log(_T("socket %08X accepted"), unsigned(client.s));
			}
		}
		else if ((e.lNetworkEvents & FD_READ) && e.iErrorCode[FD_READ_BIT] == 0)
		{
			if (!ll->onClientSockRead(client))
				server_log(_T("onClientSockRead() failed."));
		}
		else if ((e.lNetworkEvents & FD_CLOSE) && e.iErrorCode[FD_CLOSE_BIT] == 0 && index == 1)
		{
			if (!ll->onClientSockClose(client))
				server_log(_T("onClientSockClose() failed."));
		}
	}
	if (evts[1])
		WSACloseEvent(evts[1]);
	if (evts[0])
		WSACloseEvent(evts[0]);
	if (client.s != INVALID_SOCKET)
		closesocket(client.s);
	if (sockets[1] != INVALID_SOCKET)
		closesocket(sockets[1]);
	if (sockets[0] != INVALID_SOCKET)
		closesocket(sockets[0]);
	client.s = INVALID_SOCKET;
	server_log(_T("server thread is down"));
}

LRESULT CALLBACK Labeless::helperWinProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
	Labeless& ll = Labeless::instance();
	if (msg == gConfig.hlpLogMessageId)
	{
		if (const TCHAR* const m = reinterpret_cast<const TCHAR*>(wp))
		{
			xstring s(m, lp);
			delete[] m;

			std::deque<xstring> items = util::split(s);
			for (auto v : items)
			{
				log_r_no_fn("%s", v.c_str());
			}
		}
		return 0;
	}
	if (msg == gConfig.hlpCommandReceived)
	{
		ClientData& cd = ll.clientData();
		if (!ll.onCommandReceived(cd))
			log_r("onCommandReceived() failed.");

		return 0;
	}
	if (msg == gConfig.hlpPortChanged)
	{
		ll.onPortChanged();
		return 0;
	}
	switch (msg) {
	case WM_CREATE:
		return 0;
	case WM_PAINT:
		ValidateRect(hw, 0);
		return 0;
	case WM_CLOSE:
		DestroyWindow(hw);
		return 0;
	case WM_DESTROY:
		gConfig.helperWnd = nullptr;
		break;
	default:
		return DefWindowProc(hw, msg, wp, lp);
	}
	return 0;
}

bool Labeless::onCommandReceived(const std::string& command, const std::string& scriptExternObj, std::string& resultObj)
{
	bool hasException = false;
	const bool rv = safePyRunSimpleString(command, scriptExternObj, hasException, resultObj);
	if (!rv)
	{
		log_r("safePyRunSimpleString() failed. With exception: %u", int(hasException));
		if (hasException)
			PyErr_Print();
	}
	return rv;
}

bool Labeless::onCommandReceived(ClientData& cd)
{
	rpc::Response response;
	Request request;
	try
	{
		do {
			recursive_lock_guard lock(cd.commandsLock);
			if (cd.commands.empty())
				return false;
			request = cd.commands.back();
		} while (0);
		response.set_job_id(request.id);

		cd.stdOut.str("");
		cd.stdOut.clear();
		cd.stdErr.str("");
		cd.stdErr.clear();

		std::string resultObj;

		if (!onCommandReceived(request.script, request.scriptExternObj, resultObj))
		{
			log_r("An error occured");
			response.set_error("An error occurred");
		}
		response.set_std_out(cd.stdOut.str());
		response.set_std_err(cd.stdErr.str());
		if (!resultObj.empty())
			response.set_script_result_obj(resultObj);

		do {
			recursive_lock_guard lock(cd.commandsLock);
			Request* const pReq = cd.find(request.id);
			if (!pReq)
			{
				log_r("Unable to find request for job id: %llu", request.id);
				return false;
			}
			if (!pReq->error.empty())
			{
				if (response.has_error())
					pReq->error = response.error() + "\r\n" + pReq->error;
				response.set_error(pReq->error);
			}
			response.set_rpc_result(pReq->binaryResult);
			response.set_job_status(rpc::Response::JS_FINISHED);
			pReq->result = response.SerializeAsString();
			pReq->finished = true;
		} while (0);
	}
	catch (...)
	{
		log_r("exception occured");

		response.set_error("Labeless::onCommandReceived() thrown an exception");
		response.set_std_out(cd.stdOut.str());
		response.set_std_err(cd.stdErr.str());

		do {
			recursive_lock_guard lock(cd.commandsLock);
			Request* const pReq = cd.find(request.id);
			if (!pReq)
			{
				log_r("Unable to find request for job id: %llu", request.id);
				return false;
			}
			response.set_rpc_result(pReq->binaryResult);
			response.set_job_status(rpc::Response::JS_FINISHED);
			pReq->result = response.SerializeAsString();
			pReq->finished = true;
		} while (0);
	}
	return true;
}

void Labeless::onPortChanged()
{
	TCHAR buff[MAX_STRING_SIZE] = {};
	StringCchPrintf(buff, MAX_STRING_SIZE, _T("Specified port is busy. Labeless chosen an another port for you: %u"), unsigned(m_Port));
	MessageBox(g_hwndDlg, buff, _T(""), MB_ICONINFORMATION);
}

bool Labeless::onClientSockAccept(SOCKET sock, ClientData& cd)
{
	sockaddr_in ssin = {};
	int ssinLen = sizeof(ssin);
	SOCKET s = accept(sock, reinterpret_cast<sockaddr*>(&ssin), &ssinLen);
	const xstring peer = util::inetAddrToString(&ssin);

	if (peer.empty())
	{
		server_log(_T("%s: WSAAddressToString() failed. le: %08X"), util::to_xstr(__FUNCTION__).c_str(), WSAGetLastError());
		closesocket(s);
		s = INVALID_SOCKET;
		return false;
	}
	server_log(_T("%s: Connected from: %s"), util::to_xstr(__FUNCTION__).c_str(), peer.c_str());

	const xstring filterIp = Labeless::instance().m_FilterIP;

	if (!filterIp.empty() && (peer.empty() || peer.find(filterIp) == peer.npos))
	{
		server_log(_T("%s: Rejected by IP Filter: %s"), util::to_xstr(__FUNCTION__).c_str(), peer.c_str());
		closesocket(s);
		return false;
	}
	if (INVALID_SOCKET == s)
		return false;

	int bTrue = 1;
	if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&bTrue), sizeof(bTrue)))
	{
		server_log(_T("%s: setsockopt(SO_KEEPALIVE) failed. LE: %08X\n"), util::to_xstr(__FUNCTION__).c_str(), WSAGetLastError());
		return false;
	}
	const DWORD recvTimeout = 30 * 60 * 1000;
	if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&recvTimeout), sizeof(recvTimeout)))
	{
		server_log(_T("%s: setsockopt(SO_RCVTIMEO) failed. LE: %08X\n"), util::to_xstr(__FUNCTION__).c_str(), WSAGetLastError());
		return false;
	}
	if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&recvTimeout), sizeof(recvTimeout)))
	{
		server_log(_T("%s: setsockopt(SO_SNDTIMEO) failed. LE: %08X\n"), util::to_xstr(__FUNCTION__).c_str(), WSAGetLastError());
		return false;
	}

	static const tcp_keepalive keepAliveCfg{1, 30 * 60 * 1000, 2000 };
	DWORD dwDummy = 0;
	if (SOCKET_ERROR == WSAIoctl(s, SIO_KEEPALIVE_VALS, LPVOID(&keepAliveCfg), sizeof(keepAliveCfg), nullptr, 0, &dwDummy, nullptr, nullptr))
	{
		server_log(_T("%s: WSAIoctl(SIO_KEEPALIVE_VALS) failed. LE: %08X\n"), util::to_xstr(__FUNCTION__).c_str(), WSAGetLastError());
		return false;
	}
	if (INVALID_SOCKET != cd.s)
	{
		server_log(_T("%s: cd.s has valid socket %08X, closing"), __FUNCTIONW__, unsigned(cd.s));
		closesocket(cd.s);			// TODO: check this
	}
	cd.s = s;
	cd.peer = peer;
	cd.peerPort = ssin.sin_port;
	return true;
}

bool Labeless::onClientSockBufferReceived(ClientData& cd, const std::string& rawCommand)
{
#ifdef LABELESS_ADDITIONAL_LOGGING
	std::ofstream of("c:\\labeless.log", std::ios_base::app);
	if (of)
	{
		char buff[128] = {};
		_strdate_s(buff, 128);
		of << "\r\n" << std::string(buff);
		_strtime_s(buff, 128);
		of << " " << std::string(buff) << " RECV: " << rawCommand << std::endl;
		of.close();
	}
#endif // LABELESS_ADDITIONAL_LOGGING

	rpc::Execute command;
	std::string errorStr;
	try
	{
		if (!command.ParseFromString(rawCommand))
			errorStr = "Unable to parse command";
	}
	catch (...)
	{
		errorStr = "Exception occurred, unable to parse command";
	}

	Request req;
	static uint64_t req_id;
	req.id = ++req_id;

	req.script = command.script();
	req.scriptExternObj = command.script_extern_obj();
	req.params = command.rpc_request();
	req.background = command.background();

	rpc::Response response;

	do
	{
		if (req.params.empty() || !errorStr.empty())
			break;
		if (!req.script.empty())
		{
			errorStr = "RPC request can't have 'script' value";
			break;
		}

		char szJobId[256] = {};
		_snprintf_s(szJobId, _countof(szJobId), _TRUNCATE,
			"pyexcore.PyExCore.execute(%" PRIu64  ")", req.id);
		req.script = szJobId;

		if (!command.has_job_id())
			break;

		recursive_lock_guard lock(cd.commandsLock);
		Request* pReq = cd.find(command.job_id());
		if (!pReq)
		{
			errorStr = "Job not found";
			break;
		}

		if (!pReq->finished)
		{
			response.set_job_status(rpc::Response::JS_PENDING);
			break;
		}
		response.set_job_status(rpc::Response::JS_FINISHED);
		req.result = pReq->result;
		pReq = nullptr;
		cd.remove(command.job_id());
	} while (0);

	if (!errorStr.empty() || req.background || response.has_job_status())
	{
		response.set_job_id(req.id);
		if (!errorStr.empty())
			response.set_error(errorStr);

		if (req.background && !response.has_job_status())
			response.set_job_status(rpc::Response::JS_PENDING);

		if (req.result.empty())
			req.result = response.SerializeAsString();
		if (!req.background)
			req.finished = true;
	}

	do {
		recursive_lock_guard lock(cd.commandsLock);
		cd.commands.push_back(req);
	} while (0);
	server_log(_T("%s: new request pushed { jobId: %llu, bkg: %u, finished: %u }"), util::to_xstr(__FUNCTION__).c_str(),
		req.id, req.background, req.finished);

	if (!errorStr.empty() || req.finished)
		return true;

	const bool rv = TRUE == IsWindow(gConfig.helperWnd) &&
		PostMessage(gConfig.helperWnd, gConfig.hlpCommandReceived, reinterpret_cast<WPARAM>(&cd), 0);

	return rv;
}

bool Labeless::onClientSockRead(ClientData& cd)
{
	u_long ready = 0;
	if (INVALID_SOCKET == cd.s)
		return true; // socket already closed

	do
	{
		if (SOCKET_ERROR == ioctlsocket(cd.s, FIONREAD, &ready))
		{
			server_log(_T("%s: ioctlsocket(FIONREAD) failed. Error: %08X"), util::to_xstr(__FUNCTION__).c_str(), WSAGetLastError());
			return false;
		}
		if (!ready)
		{
			const std::string& rawCommand = cd.netBuff.str();
			if (rawCommand.size() < sizeof(uint64_t))
				return true;

			const uint64_t packetLen = *reinterpret_cast<const uint64_t*>(rawCommand.c_str());
			if (!packetLen)
			{
				server_log("%s: invalid packet length, sock: %" PRIu64, __FUNCTION__, cd.s);
				return false;
			}

			if (packetLen > rawCommand.size() - sizeof(uint64_t))
				return true; // chunked packet, continue receiving

			if (rawCommand.size() - sizeof(uint64_t) > packetLen)
			{
				server_log("%s: invalid packet length, sock: %" PRIu64, __FUNCTION__, cd.s);
				return false;
			}

			cd.netBuff.str("");
			cd.netBuff.clear();

			return onClientSockBufferReceived(cd, rawCommand.substr(sizeof(uint64_t)));
		}

		char* buff = nullptr;
		try
		{
			buff = new char[ready];
		}
		catch (const std::bad_alloc&)
		{
			server_log(_T("%s: Unable to allocate 0x%08X bytes memory"), util::to_xstr(__FUNCTION__).c_str(), ready);
			return false;
		}
		std::shared_ptr<void> guard { nullptr, [buff](void*){ delete [] buff; }};

		const int read = recv(cd.s, buff, ready, 0);
		server_log(_T("%s: Received %u bytes of %u."), util::to_xstr(__FUNCTION__).c_str(), read, ready);

		if (read == 0)
			return false;
		if (SOCKET_ERROR == read)
		{
			server_log(_T("%s: Error: %s"), util::to_xstr(__FUNCTION__).c_str(), wsaErrorToString().c_str());
			return false;
		}
		std::string r(buff, buff + read);
		cd.netBuff << r;
	} while (true);
	return true;
}

bool Labeless::onClientSockClose(ClientData& cd)
{
	server_log(_T("%s: socket %08X closed"), util::to_xstr(__FUNCTION__).c_str(), unsigned(cd.s));
	closesocket(cd.s);
	cd.s = INVALID_SOCKET;

	return true;
}

xstring Labeless::lastChangeTimestamp()
{
	return util::to_xstr(__TIME__) + " " + util::to_xstr(__DATE__);
}