/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "util_python.h"
#include "types.h"

// kludge to avoid link with debug version of python27
#ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
#else
#  include <Python.h>
#endif // _DEBUG

namespace {

DWORD pyExecExceptionFilter(DWORD code, _EXCEPTION_POINTERS* ep)
{
	// FIXME
	/*PyObject* msg = PyString_FromFormat("An exception occurred, code: 0x%x", code);
	stdErrHandler(nullptr, msg);
	Py_XDECREF(msg);*/
	return EXCEPTION_EXECUTE_HANDLER;
}

// grabbed from https://github.com/idapython/src
class gil_lock_t
{
private:
	PyGILState_STATE state;

private:
	gil_lock_t(const gil_lock_t&) = delete;
	gil_lock_t& operator=(const gil_lock_t&) = delete;
	gil_lock_t&& operator=(gil_lock_t&&) = delete;
public:
	gil_lock_t()
	{
		state = PyGILState_Ensure();
	}

	~gil_lock_t()
	{
		PyGILState_Release(state);
	}
};

class PyPTR
{
	PyPTR(const PyPTR&) = delete;
	PyPTR& operator=(const PyPTR&) = delete;
	PyPTR&& operator=(PyPTR&&) = delete;
public:
	explicit PyPTR(PyObject* o)
		: m_o(o)
	{
	}
	~PyPTR()
	{
		Py_XDECREF(m_o);
	}

	inline PyObject* data() { return m_o; }
	inline PyObject* data() const { return m_o; }
	PyObject* operator->() { return m_o; }
	operator bool() const { return m_o != nullptr; }
	operator PyObject* const() { return m_o; }
	bool operator==(const PyPTR& r) const { return m_o == r.m_o; }
	bool operator!=(const PyPTR& r) const { return !operator==(r); }

private:
	PyObject* m_o;
};

#define GIL_CHKCONDFAIL (((debug & IDA_DEBUG_PLUGIN) == IDA_DEBUG_PLUGIN) \
                      && PyGILState_GetThisThreadState() != _PyThreadState_Current)

#define PYW_GIL_CHECK_LOCKED_SCOPE()														\
  do																						\
  {																							\
    if ( GIL_CHKCONDFAIL )																	\
    {																						\
      msg("*** WARNING: Code at %s:%d should have the GIL, but apparently doesn't ***\n",	\
          __FILE__, __LINE__);																\
      if ( under_debugger )																	\
        BPT;																				\
    }																						\
  } while ( false )


bool safeRunStringImpl(const std::string& script, bool& exceptionOccured, std::string& error)
{
	bool rv = true;
	PYW_GIL_CHECK_LOCKED_SCOPE();

	__try
	{
		PyObject* m = PyImport_AddModule("__main__");
		if (!m)
			return false;
		PyObject* d = PyModule_GetDict(m);

		PyObject* v = PyRun_StringFlags(script.c_str(), Py_file_input, d, d, NULL);
		if (!v)
		{
			PyErr_Print();
			return false;
		}

		rv &= true;

		Py_XDECREF(v);
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

} // anonymous


namespace util {

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

	if (!PyEval_ThreadsInitialized())
		PyEval_InitThreads();

	// TODO

	//PyEval_ReleaseThread(PyThreadState_Get());

	return true;
}

bool safeRunString(const std::string& script, bool& exceptionOccured, std::string& error)
{
	exceptionOccured = false;
	error.clear();
	
	gil_lock_t gil_lock;
	Q_UNUSED(gil_lock);

	return safeRunStringImpl(script, exceptionOccured, error);
}

namespace jedi {

static PyObject* g_pyJedi;

bool init(QString& error)
{
	error.clear();

	do {
		gil_lock_t gil_lock;
		Q_UNUSED(gil_lock);

		PYW_GIL_CHECK_LOCKED_SCOPE();

		if (!g_pyJedi)
		{
			g_pyJedi = PyImport_ImportModule("jedi");
			Py_XINCREF(g_pyJedi);
		}
		if (!g_pyJedi)
		{
			error = "Cannot import `jedi` module";
			return false;
		}
	} while (0);

	// preload some IDA modules
	static const char kJedifyIdaAPI[] =
		"import idaapi\n"
		"idaapi.";

	static const char kJedifyIdaUtils[] =
		"import idautils\n"
		"idautils.";

	QStringList compls;
	::jedi::SignatureMatchList sml;

	// FIXME: precache the following modules: idaapi, idautils
	bool rv = true;
	rv &= get_completions(kJedifyIdaAPI, 1, ::qstrlen("idaapi."), compls, sml, error);
	rv &= get_completions(kJedifyIdaUtils, 1, ::qstrlen("idautils."), compls, sml, error);
	

	return g_pyJedi != nullptr && rv;
}

bool is_available()
{
	return g_pyJedi != nullptr;
}

bool getQStringFromPyUnicode(PyObject* o, QString& rv)
{
	rv.clear();

	if (!o || !PyUnicode_Check(o))
		return false;

	PyPTR pyUtf8Str{ PyUnicode_AsUTF8String(o) };
	if (!pyUtf8Str)
		return false;

	const char* cstr = PyString_AsString(pyUtf8Str);
	rv = QString::fromUtf8(cstr);
	return true;
}

QString getErrorInfo()
{
	if (!PyErr_Occurred())
		return QString::null;

	PyObject* exc = nullptr;
	PyObject* val = nullptr;
	PyObject* tb = nullptr;

	PyErr_Fetch(&exc, &val, &tb);
	QString rv;
	QString msg;
	if (val)
	{
		val = PyObject_Str(val);
		if (val && PyString_Check(val))
		{
			if (auto errStr = PyString_AsString(val))
				msg = errStr;
		}
		else if (val && PyUnicode_Check(val))
		{
			PyPTR pyUTF8str{ PyUnicode_AsUTF8String(val) };
			if (pyUTF8str && PyString_Check(pyUTF8str))
				msg = QString::fromUtf8(PyString_AsString(pyUTF8str));
		}
	}
	if (exc)
	{
		PyPTR tmp{PyObject_GetAttrString(exc, "__name__")};
		rv = PyString_AsString(tmp);
	}
	if (!msg.isEmpty())
		rv = QString("%1(%2)").arg(rv).arg(msg);
	/*if (tb)
	{
		std::string err;
		pyTraceback_AsString(tb, err);
		rv += QChar('\n') + QString::fromStdString(err);
	}*/
	return rv;
}

bool get_completions(const QString& script,
	int zline,
	int zcol,
	QStringList& completions,
	::jedi::SignatureMatchList& signatureMatches,
	QString& error)
{
	completions.clear();
	signatureMatches.clear();
	error.clear();

	gil_lock_t gil_lock;
	Q_UNUSED(gil_lock);

	PYW_GIL_CHECK_LOCKED_SCOPE();

	if (!is_available())
		return false;

	if (!g_pyJedi)
	{
		error = "Cannot import `jedi` module";
		return false;
	}

	PyObject* pyJediDict = PyModule_GetDict(g_pyJedi);
	if (!pyJediDict)
	{
		error = "Cannot get py`jedi` module's dict\n" + getErrorInfo();
		return false;
	}

	/*PyPTR pyClassDict{ PyDict_New() };
	PyPTR pyClassName{ PyBytes_FromString("jedi.Script") };
	PyPTR pyJediScriptClass{ PyClass_New(NULL, pyClassDict, pyClassName) };*/

	PyObject* pyJediScriptClass = PyDict_GetItemString(pyJediDict, "Script");
	if (!pyJediScriptClass || !PyCallable_Check(pyJediScriptClass))
	{
		error = "Cannot get py`jedi` module's Script class\n" + getErrorInfo();
		return false;
	}

	const QByteArray& ba = script.toUtf8();

	PyPTR pySource { PyString_FromStringAndSize(ba.data(), ba.size())};
	if (!pySource)
	{
		error = "Cannot create py`str` from passed script value\n" + getErrorInfo();
		return false;
	}

	PyPTR pyLine { PyInt_FromLong(zline + 1) };
	PyPTR pyColumn { PyInt_FromLong(zcol) };

	PyPTR pykwARGS { PyDict_New() };
	PyDict_SetItemString(pykwARGS, "source", pySource);
	PyDict_SetItemString(pykwARGS, "line", pyLine);
	PyDict_SetItemString(pykwARGS, "column", pyColumn);

	PyPTR pyARGS{ PyTuple_New(0) };
	PyPTR pyJediClassInstance { PyObject_Call(pyJediScriptClass, pyARGS, pykwARGS) };
	if (!pyJediClassInstance)
	{
		error = "Cannot create a `jedi.Script` instance\n" + getErrorInfo();
		return false;
	}

	PyPTR pyCompletionsFn { PyObject_GetAttrString(pyJediClassInstance, "completions") };
	if (!pyCompletionsFn)
	{
		error = "Cannot get `jedi.Script.completions function\n" + getErrorInfo();
		return false;
	}
	PyPTR pyCompletions{ PyObject_CallObject(pyCompletionsFn, NULL) };
	if (!pyCompletions || !PyList_Check(pyCompletions))
	{
		error = "Cannot get completions\n" + getErrorInfo();
		return false;
	}

	const auto completionsCnt = PyList_Size(pyCompletions);
	for (Py_ssize_t i = 0; i < completionsCnt; ++i)
	{
		PyObject* pyItem = PyList_GetItem(pyCompletions, i);
		if (!pyItem)
		{
			error = "Unable to get one of completions item\n" + getErrorInfo();
			return false;
		}

		QString completionStr;
		PyPTR pyCompletion{ PyObject_GetAttrString(pyItem, "name") };
		if (getQStringFromPyUnicode(pyCompletion, completionStr))
			completions.append(completionStr);
	}

	PyPTR pyCallSigsFn{ PyObject_GetAttrString(pyJediClassInstance, "call_signatures") };
	if (!pyCallSigsFn)
	{
		error = "Cannot get `jedi.Script.call_signatures` function\n" + getErrorInfo();
		return false;
	}

	PyPTR pyCallSigList{ PyObject_CallObject(pyCallSigsFn, NULL) };
	if (!pyCallSigList || !PyList_Check(pyCallSigList))
	{
		error = "Cannot get CALL signatures\n" + getErrorInfo();
		return false;
	}

	const auto callSigsCnt = PyList_Size(pyCallSigList);
	for (Py_ssize_t csIdx = 0; csIdx < callSigsCnt; ++csIdx)
	{
		PyObject* pyCS = PyList_GetItem(pyCallSigList, csIdx);
		if (!pyCS)
		{
			error = "Unable to get one of call signatures item\n" + getErrorInfo();
			return false;
		}

		signatureMatches.push_back({});
		::jedi::SignatureMatch& sm = signatureMatches.back();

		do {
			PyPTR pyType{ PyObject_GetAttrString(pyCS, "type") };
			getQStringFromPyUnicode(pyType, sm.type);
		} while (0);
			
		do {
			PyPTR pyName{ PyObject_GetAttrString(pyCS, "name") };
			getQStringFromPyUnicode(pyName, sm.name);
		} while (0);

		do {
			PyPTR pyIndex{ PyObject_GetAttrString(pyCS, "index") };
			if (!pyIndex)
				break;

			if (PyInt_Check(pyIndex))
				sm.argIndex = PyInt_AS_LONG(pyIndex.data());
			else if (PyLong_Check(pyIndex))
				sm.argIndex = PyLong_AsLong(pyIndex);
		} while (0);

		do {
			PyPTR pyRawDoc{ PyObject_GetAttrString(pyCS, "raw_doc") };
			if (!pyRawDoc || !PyUnicode_Check(pyRawDoc))
				break;
			PyPTR pyUtf8RawDoc{ PyUnicode_AsUTF8String(pyRawDoc) };
			if (!pyUtf8RawDoc)
				break;
			if (const char* crawDoc = PyString_AsString(pyUtf8RawDoc))
				sm.rawDoc = crawDoc;
		} while (0);

		do {
			PyPTR pyParams{ PyObject_GetAttrString(pyCS, "params") };
			if (!pyParams || !PyList_Check(pyParams.data()))
				break;
				
			const auto paramsCnt = PyList_Size(pyParams);
			for (Py_ssize_t argNum = 0; argNum < paramsCnt; ++argNum)
			{
				PyObject* pyParam = PyList_GetItem(pyParams, argNum);
				if (!pyParam)
				{
					error = "Unable to get one of signatures item params\n" + getErrorInfo();
					return false;
				}

				::jedi::FuncArg arg;

				do {
					PyPTR pyName{ PyObject_GetAttrString(pyParam, "name") };
					getQStringFromPyUnicode(pyName, arg.name);
				} while (0);

				do {
					PyPTR pyDescription{ PyObject_GetAttrString(pyParam, "description") };
					getQStringFromPyUnicode(pyDescription, arg.description);
				} while (0);

				if (!arg.name.isEmpty())
					sm.args.append(arg);
			}
		} while (0);
	}

	/*if (!pyJediClassInstance)
	{
		std::string err;
		// TODO: check for error
		PyObject *exc = 0, *val = 0, *tb = 0;

		PyErr_Fetch(&exc, &val, &tb);
		if (val)
		{
			if (auto errStr = PyString_AsString(val))
			{
				err = errStr;
			}
		}
		if (tb)
		{
			pyTraceback_AsString(tb, err);
			error = QString::fromStdString(err);
		}
		error += "\nCannot create `jedi.Script()`";
		return false;
	}*/


	//PyObject_CallFunctionObjArgs()
#if 0
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: python extlang not found\n", __FUNCTION__);
		return false;
	}

	char errbuff[1024 * 5] = {};

	if (!is_available())
	{
		//msg("%s: unable to init python code-completer, error: %s", __FUNCTION__, errbuff);
		return false;
	}

	const QByteArray& ba = script.toUtf8();
	const char* s = ba.data();

	idc_value_t idcTmp(s);
	if (!elng->set_attr(nullptr, "_src", &idcTmp))
	{
		msg("%s: set_script_attr(_src) failed", __FUNCTION__);
		return false;
	}

	static const QString kProcessScript = "_script = __jedi.Script(source=_src, line=%1, column=%2)\n"
		"_completions = [str(_.name) for _ in _script.completions()]\n"
		"_call_sigs = [{'t': str(_cs.type), 'n': str(_cs.name), 'i': _cs.index, 'args': [{'n': str(_.name), 'd': str(_.description)} for _ in _cs.params]} for _cs in _script.call_signatures()]\n"
		"del _script\n"
		"del _src\n";
	const std::string stmt = kProcessScript.arg(zline + 1).arg(zcol).toStdString();
	QElapsedTimer t;
	t.start();
	if (!elng->run_statements(stmt.c_str(), errbuff, sizeof(errbuff)))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute jedi.Script ctor, error: %s", __FUNCTION__, errbuff);
		return false;
	}
	int nMsec = t.elapsed();
	msg("%s: run_statements() took %s\n", __FUNCTION__, QDateTime::fromMSecsSinceEpoch(nMsec).toUTC().toString("hh:mm:ss.zzz").toStdString().c_str());

	t.restart();
	idc_value_t val;
	if (!elng->get_attr(nullptr, "_completions", &val) && val.vtype == VT_OBJ)
	{
		error = QObject::tr("cannot get _completions variable");
		return false;
	}

	for (const char *attr_name = VarFirstAttr(&val);
		attr_name != NULL;
		attr_name = VarNextAttr(&val, attr_name))
	{
		idc_value_t v;
		if (!VarGetAttr(&val, attr_name, &v, true) && v.vtype == VT_STR2)
			completions.append(QString::fromStdString(v.c_str()));
	}

	if (!elng->get_attr(nullptr, "_call_sigs", &val))
	{
		error = QObject::tr("cannot get _call_sigs variable");
		return false;
	}
	for (const char *idx = VarFirstAttr(&val);
		idx != NULL;
		idx = VarNextAttr(&val, idx))
	{
		signatureMatches.push_back({});
		SignatureMatch& sm = signatureMatches.back();

		idc_value_t v;
		if (VarGetAttr(&val, idx, &v, true) || v.vtype != VT_OBJ)
			continue;

		idc_value_t tmp;
		if (!VarGetAttr(&v, "t", &tmp) && tmp.vtype == VT_STR2)
			sm.type = tmp.c_str();

		if (!VarGetAttr(&v, "n", &tmp) && tmp.vtype == VT_STR2)
			sm.name = tmp.c_str();

		if (!VarGetAttr(&v, "i", &tmp))
		{
			if (tmp.vtype == VT_LONG)
				sm.argIndex = tmp.num;
			else if (tmp.vtype == VT_INT64)
				sm.argIndex = static_cast<int>(tmp.i64);
			else
				msg("%s: Invalid type of signature match's argument index: %d, expected VT_LONG or VT_INT64\n", __FUNCTION__, tmp.vtype);
		}

		idc_value_t args;
		if (!VarGetAttr(&v, "args", &args) && args.vtype == VT_OBJ)
		{
			idc_value_t param;
			for (const char* arg_idx = VarFirstAttr(&args);
				arg_idx != NULL;
				arg_idx = VarNextAttr(&args, arg_idx))
			{
				if (VarGetAttr(&args, arg_idx, &param) || param.vtype != VT_OBJ)
				{
					msg("%s: Cannot get args[%s]\n", __FUNCTION__, arg_idx);
					break;
				}

				FuncArg arg;
				if (!VarGetAttr(&param, "n", &tmp) && tmp.vtype == VT_STR2)
					arg.name = tmp.c_str();
				if (!VarGetAttr(&param, "d", &tmp) && tmp.vtype == VT_STR2)
					arg.description = tmp.c_str();

				if (!arg.name.isEmpty())
					sm.args.append(arg);
			}
		}
	}
	nMsec = t.elapsed();
	msg("%s: completions parsing took %s\n", __FUNCTION__, QDateTime::fromMSecsSinceEpoch(nMsec).toUTC().toString("hh:mm:ss.zzz").toStdString().c_str());

	elng->run_statements("del _completions\n"
		"del _call_sigs\n", errbuff, sizeof(errbuff));
#endif // 0

	return true;
}

} // jedi
} // python
} // util
