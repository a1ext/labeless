#include "jedi.h"
#include "types.h"

// IDA
#include <expr.hpp>

// Qt
#include <QDateTime>
#include <QElapsedTimer>
#include <QStringList>

// kludge to avoid link with debug version of python27
#ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
#else
#  include <Python.h>
#endif // _DEBUG


namespace jedi {

static bool gInitialized = false;


bool init_completer(const extlang_t* elng, QString& error)
{
	static const char kImportJedi[] = "import jedi as __jedi";
	char errbuff[1024] = {};

	//hlp::py

	idc_value_t val;
	if (elng->calcexpr(BADADDR, "__jedi is not None", &val, errbuff, _countof(errbuff)))
		return true;
	
	if (!elng->run_statements(kImportJedi, errbuff, _countof(errbuff)))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		return false;
	}

	gInitialized = true;
	bool rv = gInitialized;

	// preload some IDA modules
	/*static const char kJedifyIdaAPI[] =
		"import idaapi\n"
		"idaapi.";

	static const char kJedifyIdaUtils[] =
		"import idautils\n"
		"idautils.";

	QStringList compls;
	SignatureMatchList sml;
	QString err;

	rv &= get_completions(kJedifyIdaAPI, 1, ::qstrlen("idaapi."), compls, sml, err);
	rv &= get_completions(kJedifyIdaUtils, 1, ::qstrlen("idautils."), compls, sml, err);
	*/
	return rv;
}

bool is_available()
{
	return gInitialized;
}

bool get_completions(const QString& script,
	int zline,
	int zcol,
	QStringList& completions,
	SignatureMatchList& signatureMatches,
	QString& error)
{
	completions.clear();
	signatureMatches.clear();
	error.clear();

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
	const qstring& s = ba.data();

	idc_value_t idcTmp(s.c_str());
	if (!elng->set_attr(nullptr, "_src", &idcTmp))
	{
		msg("%s: set_script_attr(_src) failed", __FUNCTION__);
		return false;
	}

	static const QString kProcessScript = "_script = __jedi.Script(_src, line=%1, column=%2)\n"
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

	return true;
}

} // jedi
