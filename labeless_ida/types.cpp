/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"

#include <QObject>

ImportEntry::ImportEntry()
	: ordinal(0)
	, ea(BADADDR)
{
}

MemoryRegion::MemoryRegion(uint64_t base_, uint64_t size_, quint32 protect_, bool forceProtect_)
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

#if (IDA_SDK_VERSION >= 930)
// fix of broken 9.3 sdk
uint128 operator<<(const uint128& x, int cnt)
{
	uint64 l = x.l << cnt;
	uint64 h = (x.h << cnt) | (x.l >> (64 - cnt));
	return uint128(l, h);
}
#endif // (IDA_SDK_VERSION >= 930)
