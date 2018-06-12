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
#include "util_python.h"

// IDA
#include <expr.hpp>

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

	const std::string stmt = kResultKeyword + " = json.loads(" + kResultStrKeyword + ")";
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
} // idapython
} // util
