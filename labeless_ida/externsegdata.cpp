/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "externsegdata.h"


ExternSegData::ExternSegData()
{
}

ImportEntry* ExternSegData::addAPI(const std::string& module, const std::string& proc, uint64_t ea, uint64_t ordinal /*= 0*/)
{
	const std::string joined = module + "." + proc;

	ImportEntry& ie = imports[joined];
	ie.module = module;
	ie.proc = proc;
	ie.ordinal = ordinal;
	ie.ea = ea;

	return &imports[joined];
}
