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

MemoryRegion::MemoryRegion(uint64_t base_, uint64_t size_, uint32 protect_, bool forceProtect_)
	: base(base_)
	, size(size_)
	, protect(protect_)
	, forceProtect(forceProtect_)
{
}

bool MemoryRegion::isIntersects(const MemoryRegion& r) const
{
	return base <= r.end() && r.base <= end();
}

Settings::Settings(const std::string host_,
	uint16_t port_,
	bool enabled_,
	bool demangle_,
	bool localLabels_,
	bool nonCodeNames_,
	bool analysePEHeader_,
	bool postProcessFixCallJumps_,
	bool removeFuncArgs_,
	OverwriteWarning overwriteWarning_,
	CommentSyncFlags commentsSync_,
	bool codeCompletion_)
	: host(host_)
	, port(port_)
	, enabled(enabled_)
	, demangle(demangle_)
	, localLabels(localLabels_)
	, nonCodeNames(nonCodeNames_)
	, analysePEHeader(analysePEHeader_)
	, postProcessFixCallJumps(postProcessFixCallJumps_)
	, removeFuncArgs(removeFuncArgs_)
	, overwriteWarning(overwriteWarning_)
	, commentsSync(commentsSync_)
	, codeCompletion(codeCompletion_)
{
}

ScopedEnabler::ScopedEnabler(QAtomicInt& ref_)
	: ref(ref_)
{
	ref = 1;
}

ScopedEnabler::~ScopedEnabler()
{
	ref = 0;
}

ScopedSignalBlocker::ScopedSignalBlocker(const QList<QPointer<QObject>>& items_)
	: items(items_)
{
	for (int i = 0; i < items.length(); ++i)
		if (items.at(i))
			items.at(i)->blockSignals(true);
}

ScopedSignalBlocker::~ScopedSignalBlocker()
{
	for (int i = 0; i < items.length(); ++i)
		if (items.at(i))
			items.at(i)->blockSignals(false);
}

ScopedWaitBox::ScopedWaitBox(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	show_wait_box_v(fmt, va);
	va_end(va);
}

ScopedWaitBox::~ScopedWaitBox()
{
	hide_wait_box();
}
