/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#ifdef __NT__
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <WinSock2.h>
#define FORCE_ENUM_SIZE_INT

#elif defined(__unix__) || defined(__linux__)

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOPY         0x08
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD            0x100
#define PAGE_NOCACHE          0x200
#define PAGE_WRITECOMBINE     0x400

#define SEC_IMAGE         0x1000000

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

#define _countof(X) (sizeof((X))/sizeof((X)[0]))
#define FORCE_ENUM_SIZE_INT : int
#endif

#ifdef __NT__
#pragma warning(push)
#pragma warning(disable:4309)           // disable "truncation of constant value" warning from IDA SDK
#endif // __NT__
#include <pro.h>
#ifdef __NT__
#pragma warning(pop)
#endif // __NT__
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

QT_FORWARD_DECLARE_CLASS(QObject);

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
	bool forceProtect; // not serializable

	MemoryRegion(uint64_t base_, uint64_t size_, uint32 protect_, bool forceProtect_ = false);

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
		CS_Disabled				= 0,
		CS_IDAComment			= 1 << 0, // means repeatable and non-repeatable
		CS_LocalVar				= 1 << 1,
		CS_FuncNameAsComment	= 1 << 2
	};
	Q_DECLARE_FLAGS(CommentSyncFlags, CommentsSync);

	std::string host;
	uint16_t port;
	uint64_t remoteModBase;
	bool enabled;
	bool demangle;
	bool localLabels;
	bool nonCodeNames;
	bool analysePEHeader;
	bool postProcessFixCallJumps;
	bool removeFuncArgs;
	OverwriteWarning overwriteWarning;
	CommentSyncFlags commentsSync;

	Settings(const std::string host_ = std::string(),
		uint16_t port_ = 0,
		bool enabled_ = false,
		bool demangle_ = false,
		bool localLabels_ = false,
		bool nonCodeNames_ = false,
		bool analysePEHeader_ = false,
		bool postProcessFixCallJumps_ = false,
		bool removeFuncArgs_ = false,
		OverwriteWarning overwriteWarning_ = OW_AlwaysAsk,
		CommentSyncFlags commentsSync_ = CS_Disabled);
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

struct ScopedEnabler
{
	QAtomicInt& ref;
	ScopedEnabler(QAtomicInt& ref_);
	virtual ~ScopedEnabler();
};

struct ScopedSignalBlocker
{
	QList<QPointer<QObject>> items;
	ScopedSignalBlocker(const QList<QPointer<QObject>>& items_);
	virtual ~ScopedSignalBlocker();
};

struct ScopedWaitBox
{
	explicit ScopedWaitBox(const char* fmt, ...);
	~ScopedWaitBox();
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

inline ea_t targetPtrSize()
{
	return ::inf.is_64bit() ? sizeof(uint64_t) : sizeof(uint32_t);
}

typedef std::unordered_map<uint64_t, std::string> ExternRefDataMap;

#ifdef __NT__
#define CHECKED_CONNECT(X)											\
	if (!(X)) do {													\
		msg(__FUNCTION__ ": connect() failed: " #X "\n");			\
	} while (0)
#elif defined(__unix__) || defined(__linux__)
// FIXME
#define CHECKED_CONNECT(X)                                          \
    if (!(X)) do {													\
	} while (0)
#endif

#define OLLY_TEXTLEN 256

#ifdef _WIN64
#	define MY_PFX_PTR  "ll"
#else
#	define MY_PFX_PTR  "l"
#endif
#define MY_PRIXPTR      MY_PFX_PTR "X"


#ifdef __X64__
#	define LL_FMT_EA_T  "llX"
#else
#	define LL_FMT_EA_T  "X"
#endif


#if !defined(__NT__) && !defined(__unix__) && !defined(__unix__)
#error "WIN32 and Linux/Unix platforms are supported"
#endif
