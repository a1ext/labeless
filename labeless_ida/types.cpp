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
	, ea(BADADDR)
{
}

MemoryRegion::MemoryRegion(uint64_t base_, uint64_t size_, uint32 protect_)
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
	uint64_t remoteModBase_,
	bool enabled_,
	bool demangle_,
	bool localLabels_,
	bool nonCodeNames_,
	bool analysePEHeader_,
	bool postProcessFixCallJumps_,
	OverwriteWarning overwriteWarning_,
	CommentSyncFlags commentsSync_)
	: host(host_)
	, port(port_)
	, enabled(enabled_)
	, demangle(demangle_)
	, localLabels(localLabels_)
	, nonCodeNames(nonCodeNames_)
	, analysePEHeader(analysePEHeader_)
	, postProcessFixCallJumps(postProcessFixCallJumps_)
	, overwriteWarning(overwriteWarning_)
	, commentsSync(commentsSync_)
{
}
