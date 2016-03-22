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

#include <pro.h>
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

enum ControlID
{
	CID_Script					= 1,
	CID_Log						= 2,
	CID_RunButton				= 3,
	CID_SettingsButton			= 4,
	CID_WipeAndReconstruct		= 5,
	CID_ClearLog				= 6,
	CID_ShowAllResponsesInLog	= 7
};

struct ICommand;
typedef std::shared_ptr<ICommand> ICommandPtr;

struct PythonPalette;

namespace rpc {
class Response;
} // rpc


class RpcData;
typedef QPointer<RpcData> RpcDataPtr;

typedef std::function<bool(RpcDataPtr)> RpcReadyToSendHandler;



struct ImportEntry
{
	std::string module;
	std::string proc;
	uint64_t ordinal;
	//intptr_t val;
	uint32_t index;

	ImportEntry();
};

struct ExternSegData
{
	typedef std::unordered_map<std::string, ImportEntry> ImportsMap;

	uint64_t	start;
	uint64_t	len;
	ImportsMap	imports;

	ExternSegData();
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
	uint64_t defaultExternSegSize;
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
		uint64_t defaultExternSegSize = 0,
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

typedef std::unordered_map<uval_t, std::string> ExternRefDataMap;

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
#		define LL_FMT_EA_T  "ulX"
#	endif
#endif // defined(_MSC_VER) || defined(__MINGW32__)

#ifndef __NT__
#error "Only Win32 platform is supported"
#endif // __NT__
