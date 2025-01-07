/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "util_idapython.h"
#include "types.h"
//#include "../jedi.h"
#include "../../common/version.h"

// IDA
#if defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif // defined(__GNUC__)
#include <expr.hpp>
#if defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif // defined(__GNUC__)


namespace util {

namespace idapython {

static const std::string kExternKeyword = "__extern__";
static const std::string kResultKeyword = "__result__";
static const std::string kResultStrKeyword = "__result_str__";

static const char kNameError[] = "NameError";


bool init()
{
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: python extlang not found\n", __FUNCTION__);
		return false;
	}
	static const std::string pyInitMsg = "import json;" + kResultKeyword + " = None\n"
		"idaapi.msg('Labeless: Python initialized... OK\\n')\n";
#if (IDA_SDK_VERSION < 700)
	char errbuff[MAXSTR] = {};
	if (!run_statements(pyInitMsg.c_str(), errbuff, _countof(errbuff), elng))
	{
		msg("%s: run_statements() failed\n", __FUNCTION__);
		if (::qstrlen(errbuff))
			msg("%s: error: %s", __FUNCTION__, errbuff);
		return false;
	}
#else // IDA_SDK_VERSION < 700
	::qstring errbuff;
	if (!elng->eval_snippet(pyInitMsg.c_str(), &errbuff))
	{
		msg("%s: run_statements() failed\n", __FUNCTION__);
	
		if (!errbuff.empty())
			msg("%s: error: %s", __FUNCTION__, errbuff.c_str());
		return false;
	}
#endif // IDA_SDK_VERSION

	/*QString error;
	if (!util::python::jedi::init(error))
	{
		msg("Labeless: Unable to import `jedi` python module, auto-completion and intellisence won't be available in the python editors\n");
	}*/

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

	::qstring qerrbuff;
#if (IDA_SDK_VERSION < 700)
	if (!run_statements(script.c_str(), errbuff, _countof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}
#else // IDA_SDK_VERSION < 700
	if (!elng->eval_snippet(script.c_str(), &qerrbuff))
	{
		if (!qerrbuff.empty())
			error = qerrbuff.c_str();
		msg("%s: unable to execute Python script, error: %s\n", __FUNCTION__, qerrbuff.c_str());
		return false;
	}
#endif
	const std::string pyJsonDumpsStmt = "json.dumps(" + kExternKeyword + ") if '" + kExternKeyword + "' in globals() else ''";
	idc_value_t rv;
#if (IDA_SDK_VERSION < 700)
	if (elng->calcexpr(BADADDR, pyJsonDumpsStmt.c_str(), &rv, errbuff, sizeof(errbuff)))
	{
		externObj = rv.c_str();
	}
#else // IDA_SDK_VERSION < 700
	elng->eval_snippet("import json", &qerrbuff);
	if (elng->eval_expr(&rv, BADADDR, pyJsonDumpsStmt.c_str(), &qerrbuff))
	{
		externObj = rv.c_str();
	}
#endif // IDA_SDK_VERSION < 700
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains(kNameError) ||
		qerrbuff.find(kNameError) != qerrbuff.npos)
	{
		if (!qerrbuff.empty())
			error = qerrbuff.c_str();
		else
			error = errbuff;
		return false;
	}
	return true;
}

bool setResultObject(const std::string& obj, std::string& error)
{
	auto elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}

	select_extlang(elng);

	error.clear();

	idc_value_t varStr(obj.c_str());

#if (IDA_SDK_VERSION < 700)
	if (!elng->set_attr(nullptr, kResultStrKeyword.c_str(), &varStr))
#else // IDA_SDK_VERSION < 700
	if (!elng->set_attr(nullptr, kResultStrKeyword.c_str(), varStr))
#endif // IDA_SDK_VERSION < 700
	{
		error = "unable to set " + kResultStrKeyword;
		return false;
	}

	const std::string stmt = "import json\n" + kResultKeyword + " = json.loads(" + kResultStrKeyword + ")";
#if (IDA_SDK_VERSION < 700)
	char errbuff[1024] = {};
	if (!run_statements(stmt.c_str(), errbuff, _countof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}
#else // IDA_SDK_VERSION < 700
	::qstring errbuff;
	if (!elng->eval_snippet(stmt.c_str(), &errbuff))
	{
		if (!errbuff.empty())
			error = errbuff.c_str();
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff.c_str());
		return false;
	}
#endif // IDA_SDK_VERSION < 700
	return true;
}

namespace github {

static const std::string kHost = "api.github.com";
static const std::string kPath = "/repos/a1ext/labeless/releases/latest";
static const std::string kPyCheckScript = "import json\r\n"
	"from http.client import HTTPSConnection\r\n"
	"c = HTTPSConnection('" + kHost + "')\r\n"
	"c.request('GET', '" + kPath + "', headers={'User-Agent': '" + LABELESS_PRODUCT + "'})\r\n"
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
	::qstring qerrbuff;

#if (IDA_SDK_VERSION < 700)
	if (!run_statements(kPyCheckScript.c_str(), errbuff, sizeof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}
#else
	if (!elng->eval_snippet(kPyCheckScript.c_str(), &qerrbuff))
	{
		if (!qerrbuff.empty())
			error = qerrbuff.c_str();
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, qerrbuff.c_str());
		return false;
	}
#endif


	idc_value_t val;
#if (IDA_SDK_VERSION < 700)
	if (elng->calcexpr(BADADDR, kGetTagSTMT.c_str(), &val, errbuff, sizeof(errbuff)))
	{
		ri.tag = val.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains(kNameError))
	{
		error = errbuff;
		return false;
	}
#else // IDA_SDK_VERSION < 700
	if (elng->eval_expr(&val, BADADDR, kGetTagSTMT.c_str(), &qerrbuff))
	{
		ri.tag = val.c_str();
	}
	else if (qerrbuff.find(kNameError) != qerrbuff.npos)
	{
		error = qerrbuff.c_str();
		return false;
	}
#endif // IDA_SDK_VERSION < 700

#if (IDA_SDK_VERSION < 700)
	if (elng->calcexpr(BADADDR, kGetTagNameSTMT.c_str(), &val, errbuff, sizeof(errbuff)))
	{
		ri.name = val.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains(kNameError))
	{
		error = errbuff;
		return false;
	}
#else
	if (elng->eval_expr(&val, BADADDR, kGetTagNameSTMT.c_str(), &qerrbuff))
	{
		ri.name = val.c_str();
	}
	else if (qerrbuff.find(kNameError) != qerrbuff.npos)
	{
		error = qerrbuff.c_str();
		return false;
	}
#endif

#if (IDA_SDK_VERSION < 700)
	if (elng->calcexpr(BADADDR, kGetUrlSTMT.c_str(), &val, errbuff, sizeof(errbuff)))
	{
		ri.url = val.c_str();
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains(kNameError))
	{
		error = errbuff;
		return false;
	}
#else // IDA_SDK_VERSION < 700
	if (elng->eval_expr(&val, BADADDR, kGetUrlSTMT.c_str(), &qerrbuff))
	{
		ri.url = val.c_str();
	}
	else if (qerrbuff.find(kNameError) != qerrbuff.npos)
	{
		error = errbuff;
		return false;
	}
#endif // IDA_SDK_VERSION < 700
	return true;
}

} // github

namespace jedi {

static bool g_jediAvailable = false;

bool init(QString& error)
{
	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}
	::qstring qerrbuff;

	if (!elng->eval_snippet("import jedi\r\n_sjedi = str(jedi)\r\n", &qerrbuff))
	{
		if (!qerrbuff.empty())
			error = qerrbuff.c_str();
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, qerrbuff.c_str());
		return false;
	}

	idc_value_t val;
	if (elng->eval_expr(&val, BADADDR, "_sjedi", &qerrbuff))
	{
		if (!val.qstr().empty())
			return g_jediAvailable = true;
	}
	else if (qerrbuff.find(kNameError) != qerrbuff.npos)
	{
		error = qerrbuff.c_str();
		return false;
	}

	// preload some IDA modules
	/*static const char kJedifyIdaAPI[] =
		"import idaapi\n"
		"idaapi.";

	static const char kJedifyIdaUtils[] =
		"import idautils\n"
		"idautils.";

	QStringList compls;
	::jedi::SignatureMatchList sml;

	// FIXME: precache the following modules: idaapi, idautils
	bool rv = true;
	rv &= get_completions(kJedifyIdaAPI, 1, static_cast<int>(::qstrlen("idaapi.")), compls, sml, error);
	rv &= get_completions(kJedifyIdaUtils, 1, static_cast<int>(::qstrlen("idautils.")), compls, sml, error);


	return g_pyJedi != nullptr && rv;*/
	error = "error not supported, ask author to check";
	return false;
}

bool is_available() {
	return g_jediAvailable;
}

bool get_completions(const QString& script,
	int line,
	int col,
	::jedi::CompletionType ct,
	QStringList& completions,
	::jedi::SignatureMatchList& signatureMatches,
	QString& error) {

	if (!is_available() || ct == ::jedi::CT_Unknown)
		return false;


	const extlang_t* elng = find_extlang_by_name(PYTHON_EXTLANG_NAME);
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}
	::qstring qerrbuff;

	/*::idc_value_t globals;
	if (!elng->eval_snippet("_gg = globals()", &qerrbuff)) {
		msg("%s: get globals failed\n", __FUNCTION__);
		return false;
	}
	
	if (!elng->eval_expr(&globals, BADADDR, "_gg", &qerrbuff)) {
		msg("%s: get globals second failed\n", __FUNCTION__);
		return false;
	}*/
	::idc_value_t scriptAlias(script.toUtf8().data());
	if (!elng->set_attr(nullptr, "JEDI_COMP_SCRIPT", scriptAlias)) {
		msg("%s: set script alias failed\n", __FUNCTION__);
		return false;
	}

	::idc_value_t idc_completions, idc_signature;
	/*if (!elng->eval_expr(&rv, BADADDR, "print(JEDI_COMP_SCRIPT)", &qerrbuff)) {
		msg("%s: failed to print script\n", __FUNCTION__);
		return false;
	}*/
	
	static const char kCompletionScript[]  = R"EOF(
import jedi
_script = jedi.Script(JEDI_COMP_SCRIPT)
if %3:
    _completions = ','.join([comp.name for comp in _script.complete(line=%1, column=%2)])
else:
    _sigs = _script.get_signatures(line=%1, column=%2)
    if _sigs: 
        _sig = _sigs[0]
        _sig = {'type': _sig.type, 'name': _sig.name, 'index': _sig.index, 'doc': _sig.docstring(), 'params': [{'name': param.name, 'desc': param.description} for param in _sig.params]}
    else:
        _sig = False

)EOF";

	const QString& snip = QString(kCompletionScript)
		.arg(line + 1)
		.arg(col)
		.arg(ct == ::jedi::CT_Completions ? 1 : 0);

	if (!elng->eval_snippet(snip.toUtf8().data(), &qerrbuff)) {
		msg("%s: failed to eval jedi: %s\n", __FUNCTION__, qerrbuff.c_str());
		return false;
	}
		
	
	switch(ct) {
	case ::jedi::CT_Completions:
		if (!elng->eval_expr(&idc_completions, BADADDR, "_completions", &qerrbuff)) {
			msg("%s: failed to get jedi completions, %s\n", __FUNCTION__, qerrbuff.c_str());
			return false;
		}
		completions = QString(idc_completions.qstr().c_str()).split(',');
		return true;
	
	case ::jedi::CT_CallSignature:
		if (!elng->eval_expr(&idc_signature, BADADDR, "_sig", &qerrbuff)) {
			msg("%s: failed to get jedi signature, %s\n", __FUNCTION__, qerrbuff.c_str());
			return false;
		}

		if (idc_signature.vtype != VT_OBJ)
			break;
		idc_signature.clear();

		do {
			signatureMatches.push_back(::jedi::SignatureMatch());
			::jedi::SignatureMatch& sm = signatureMatches.back();

			::idc_value_t tmp;
			if (!elng->eval_expr(&tmp, BADADDR, "_sig['name']", &qerrbuff)) {
				msg("%s: failed to get jedi signature name\n", __FUNCTION__);
				return false;
			}
			sm.name = tmp.qstr().c_str();
			tmp.clear();

			
			if (!elng->eval_expr(&tmp, BADADDR, "_sig['type']", &qerrbuff)) {
				msg("%s: failed to get jedi signature type\n", __FUNCTION__);
				return false;
			}
			sm.type = tmp.qstr().c_str();
			tmp.clear();

			if (!elng->eval_expr(&tmp, BADADDR, "_sig['index']", &qerrbuff)) {
				msg("%s: failed to get jedi signature type\n", __FUNCTION__);
				return false;
			}
			sm.argIndex = tmp.num;
			tmp.clear();

			if (!elng->eval_expr(&tmp, BADADDR, "_sig['doc']", &qerrbuff)) {
				msg("%s: failed to get jedi signature doc\n", __FUNCTION__);
				return false;
			}
			sm.rawDoc = tmp.qstr().c_str();
			tmp.clear();

			if (!elng->eval_expr(&tmp, BADADDR, "len(_sig['params'])", &qerrbuff)) {
				msg("%s: failed to get jedi signature len params\n", __FUNCTION__);
				return false;
			}

			const int paramsCnt = tmp.num;
			tmp.clear();

			for (int i = 0; i < paramsCnt; ++i) {
				if (!elng->eval_expr(&tmp, BADADDR, QString("_sig['params'][%1]['name']").arg(i).toUtf8().data(), &qerrbuff)) {
					msg("%s: failed to get jedi signature params %d name\n", __FUNCTION__, i);
					return false;
				}
				::jedi::FuncArg arg;
				arg.name = tmp.qstr().c_str();
				tmp.clear();

				if (!elng->eval_expr(&tmp, BADADDR, QString("_sig['params'][%1]['desc']").arg(i).toUtf8().data(), &qerrbuff)) {
					msg("%s: failed to get jedi signature params %d desc\n", __FUNCTION__, i);
					return false;
				}
				arg.description = tmp.qstr().c_str();
				tmp.clear();
				sm.args.append(arg);
			}
			return true;
		} while (0);
		break;
	
	default:
		msg("%s: completion type is not supported: %d\n", __FUNCTION__, (int)ct);
	}

	return false;
}

} // jedi
} // idapython
} // util
