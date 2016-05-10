/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include "types.h"


struct ExternSegData
{
	typedef std::unordered_map<std::string, ImportEntry> ImportsMap;

	ImportsMap	imports;

	ExternSegData();

	ImportEntry* addAPI(const std::string& module, const std::string& proc, uint64_t ea, uint64_t ordinal = 0);
};
