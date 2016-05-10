/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <WinSock2.h>

#pragma warning(push)
#pragma warning(disable:4309)           // disable "truncation of constant value" warning from IDA SDK
#include <pro.h>
#pragma warning(pop)
#include <ida.hpp>
#include <idp.hpp>

#undef wait // Stupid undef 'wait' (added by IDA SDK) to allow to call m_QueueCond.wait(lock)

#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

#include <QSharedPointer>
#include <QPointer>
#include <QList>


struct ICommand;
typedef std::shared_ptr<ICommand> ICommandPtr;

struct PythonPalette;

namespace rpc {
class Response;
} // rpc


class RpcData;
struct ExternSegData;
typedef QPointer<RpcData> RpcDataPtr;

typedef std::function<bool(RpcDataPtr)> RpcReadyToSendHandler;



struct ImportEntry
{
	std::string module;
	std::string proc;
	uint64_t ordinal;
	//intptr_t val;

	uint64_t ea;

	ImportEntry();

	inline bool isValid() const { return !module.empty() && (!proc.empty() || ordinal); }
};

struct ExportItem
{
	uint64_t ea;
	uint64_t ordinal;
	std::string name;
};

struct Section
{
	std::string name;
	uint64_t va;
	uint64_t va_size;
	uint64_t raw;
	uint64_t raw_size;
	uint32_t characteristics;
};

typedef QList<ExportItem> ExportItemList;
typedef QList<Section> SectionList;

struct MemoryRegion
{
	uint64_t base;
	uint64_t size;
	uint32 protect;
	std::string name;

	MemoryRegion(uint64_t base_, uint64_t size_, uint32 protect_);

	inline uint64_t end() const {
		return base + size;
	}
	bool isIntersects(const MemoryRegion& r) const;
};
typedef QList<MemoryRegion> MemoryRegionList;

struct ReadMemoryRegions;
struct CheckPEHeaders;
struct AnalyzeExternalRefs;

struct Settings
{
	enum OverwriteWarning
	{
		OW_AlwaysAsk = 0,
		OW_Overwrite,
		OW_TakeSnapshotAndOverwrite,
		OW_WarnAndCancel
	};

	enum CommentsSync
	{
		CS_Disabled = 0,
		CS_NonRepeatable,
		CS_Repeatable,
		CS_All
	};

	std::string host;
	uint16_t port;
	uint64_t remoteModBase;
	bool enabled;
	bool demangle;
	bool localLabels;
	bool nonCodeNames;
	bool analysePEHeader;
	bool postProcessFixCallJumps;
	OverwriteWarning overwriteWarning;
	CommentsSync commentsSync;

	Settings(const std::string host_ = std::string(),
		uint16_t port = 0,
		uint64_t remoteModBase = 0,
		bool enabled = false,
		bool demangle = false,
		bool localLabels = false,
		bool nonCodeNames = false,
		bool analysePEHeader = false,
		bool postProcessFixCallJumps = false,
		OverwriteWarning overwriteWarning_ = OW_AlwaysAsk,
		CommentsSync commentsSync_ = CS_Disabled);
};

struct LogItem
{
	uint64_t number;

	uint64_t jobId;
	QString ollyScript;
	QString idaScript;
	QString textStdOut;
	QString textStdErr;
};

#if _MSC_VER >= 1800

template <typename T>
qlist<T> qlistFromInitializerList(const std::initializer_list<T>& il)
{
	qlist<T> rv;
	for (auto it = il.begin(); it != il.end(); ++it)
		rv.push_back(*it);
	return rv;
}
#endif // _MSC_VER >= 1800

inline uint64_t targetPtrSize()
{
	return ::inf.is_64bit() ? sizeof(uint64_t) : sizeof(uint32_t);
}

typedef std::unordered_map<uint64_t, std::string> ExternRefDataMap;

#define CHECKED_CONNECT(X)											\
	if (!(X)) do {													\
		msg(__FUNCTION__ ": connect() failed: " #X "\n");			\
	} while (0)

#define OLLY_TEXTLEN 256

#ifndef PRIXPTR
#	ifdef _WIN64
#		define _PFX_PTR  "ll"
#	else
#		define _PFX_PTR  "l"
#	endif
#	define PRIXPTR      _PFX_PTR "X"
#endif // PRIXPTR

#if defined(_MSC_VER) || defined(__MINGW32__)
#	ifdef __X64__
#		define LL_FMT_EA_T  "llX"
#	else
#		define LL_FMT_EA_T  "X"
#	endif
#endif // defined(_MSC_VER) || defined(__MINGW32__)

#ifndef __NT__
#error "WIN32 platform only is supported"
#endif // __NT__
