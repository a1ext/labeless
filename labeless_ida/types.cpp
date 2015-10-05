/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"

ImportEntry::ImportEntry()
	: ordinal(0)
	, index(0)
{
}

ExternSegData::ExternSegData()
	: start(0)
	, len(0)
{
}

MemoryRegion::MemoryRegion(ea_t base_, uint32 size_, uint32 protect_)
	: base(base_)
	, size(size_)
	, protect(protect_)
{
}

bool MemoryRegion::isIntersects(const MemoryRegion& r) const
{
	return base <= r.end() && r.base <= end();
}

Settings::Settings(const std::string host_,
	uint16_t port_,
	uint32_t remoteModBase_,
	bool enabled_,
	bool demangle_,
	bool localLabels_,
	bool nonCodeNames_,
	bool analysePEHeader_,
	bool postProcessFixCallJumps_,
	uint32_t defaultExternSegSize_,
	OverwriteWarning overwriteWarning_)
	: host(host_)
	, port(port_)
	, enabled(enabled_)
	, demangle(demangle_)
	, localLabels(localLabels_)
	, nonCodeNames(nonCodeNames_)
	, analysePEHeader(analysePEHeader_)
	, postProcessFixCallJumps(postProcessFixCallJumps_)
	, defaultExternSegSize(defaultExternSegSize_)
	, overwriteWarning(overwriteWarning_)
{
}
