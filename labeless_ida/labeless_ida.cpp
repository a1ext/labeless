/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "types.h"
#include "labeless_ida.h"

// IDA
#pragma warning(push)
#pragma warning(disable: 4267) // disable warning like "sdk\include\typeinf.hpp(2642): warning C4267: 'return': conversion from 'size_t' to 'cm_t', possible loss of data"
#include <ida.hpp>
#include <idp.hpp>

#include <auto.hpp>
#include <allins.hpp>
#include <bytes.hpp>
#include <dbg.hpp>
#include <demangle.hpp>
#include <entry.hpp>
#include <enum.hpp>
#include <fixup.hpp>
#include <frame.hpp>
#include <kernwin.hpp>
#include <loader.hpp>
#include <nalt.hpp>
#include <name.hpp>
#include <offset.hpp>
#include <segment.hpp>
#include <srarea.hpp>
#include <struct.hpp>
#include <typeinf.hpp>
#include <../ldr/idaldr.h>
#pragma warning(pop)

// std
#include <algorithm>
#include <deque>
#include <set>
#include <sstream>
#include <unordered_map>

// Qt
#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QRegExp>
#include <QSettings>
#include <QThread>
#include <QTextCodec>
#include <QVariant>

#include <google/protobuf/stubs/common.h>

#if defined(__NT__)
#	include <mstcpip.h>
#elif defined(__unix__) || defined(__linux__)
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <netinet/tcp.h>
#endif // defined(__unix__) || defined(__linux__)

#include "util/util_ida.h"
#include "util/util_idapython.h"
#include "util/util_net.h"
#include "rpcthreadworker.h"
#include "../common/cpp/rpc.pb.h"
#include "../common/version.h"
#include "choosememorydialog.h"
#include "globalsettingsmanager.h"
#include "idadump.h"
#include "idastorage.h"
#include "pyollyview.h"
#include "pythonpalettemanager.h"
#include "settingsdialog.h"
#include "jedi.h"
#include "jedicompletionworker.h"


namespace {


enum SettingsCtrl
{
	SC_Ip					= 1,
	SC_Port					= 2,
	SC_RemoteModuleBase		= 3,
	SC_CheckboxGroup		= 4,
	SC_TestConnectionBtn	= 5,
	SC_SyncNowBtn			= 6,
	SC_RemotePyConsole		= 7
};

static const Settings kDefaultSettings(
	"127.0.0.1",
	3852,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	Settings::OW_AlwaysAsk,
	Settings::CS_Disabled
);

enum LabelessNodeAltType
{
	LNAT_Port = 0,
	LNAT_RmoteModBase,
	LNAT_Demangle,
	LNAT_LocalLabels,
	LNAT_CommentsSync
};

void protobufLogHandler(::google::protobuf::LogLevel level, const char* filename, int line,	const std::string& message)
{
	static const QString kPrefix = "protobuf";
	if (level < ::google::protobuf::LOGLEVEL_WARNING)
		return;

	QString levelStr;
	switch (level)
	{
	case google::protobuf::LOGLEVEL_INFO:
		levelStr = "[INFO] ";
		break;
	case google::protobuf::LOGLEVEL_WARNING:
		levelStr = "[WARN] ";
		break;
	case google::protobuf::LOGLEVEL_ERROR:
		levelStr = "[ERRO] ";
		break;
	case google::protobuf::LOGLEVEL_FATAL:
		levelStr = "[FATA] ";
		break;
	default:
		break;
	}
	const QString msg = QString("%1 %2:%3 %4")
			.arg(levelStr)
			.arg(QString::fromLatin1(filename))
			.arg(line)
			.arg(QString::fromStdString(message));

	QMetaObject::invokeMethod(&Labeless::instance(), "onLogMessage", Qt::QueuedConnection,
		Q_ARG(QString, msg),
		Q_ARG(QString, kPrefix));
}

static struct SyncAllNowActionHandler_t : public action_handler_t
{
	virtual int idaapi activate(action_activation_ctx_t *)
	{
		if (Labeless::instance().isEnabled())
			QMetaObject::invokeMethod(&Labeless::instance(), "onSyncronizeAllRequested", Qt::QueuedConnection);
		return 1;
	}

	virtual action_state_t idaapi update(action_update_ctx_t *)
	{
		return AST_ENABLE_ALWAYS;
	}
} g_SyncAllNowActionHandler;


class PingThread : public QThread
{
	std::string host;
	uint16_t port;
public:
	PingThread(const std::string& host_, uint16_t port_)
		: host(host_)
		, port(port_)
	{}

	void run() override
	{
		QString error;
		bool ok = Labeless::testConnect(host, port, error);
		QMetaObject::invokeMethod(&Labeless::instance(), "onTestConnectFinished", Qt::QueuedConnection,
			Q_ARG(bool, ok),
			Q_ARG(QString, error));
	}
};

static const QString kLabelessTitle = "Labeless";
static const std::string kAPIEnumName = "OLD_API_EXTERN_CONSTS";
static const qstring kReturnAddrStackStructFieldName = " r";
static const QString kLabelessMenuObjectName = "labeless_menu";
static const QString kLabelessMenuLoadStubItemName = "act-load-stub-x86";
static const QString kLabelessMenuLoadStubItemNameX64 = "act-load-stub-x64";
static const std::string kSyncAllNowActionName = "Labeless: Sync all now";


} // anonymous

TForm* Labeless::m_EditorTForm;

Labeless::Labeless()
	: m_Initialized(false)
	, m_ConfigLock(QMutex::Recursive)
	, m_SynchronizeAllNow(false)
	, m_LabelSyncOnRenameIfZero(0)
	, m_ShowAllResponsesInLog(true)
	, m_ThreadLock(QMutex::Recursive)
	, m_AutoCompletionThreadLock(QMutex::Recursive)
	, m_AutoCompletionState(new jedi::State)
{
	msg("%s\n", __FUNCTION__);
#ifdef __NT__
	WSADATA wd = {};
	WSAStartup(MAKEWORD(2, 2), &wd);
#endif // __NT__
	m_Settings = kDefaultSettings;

	::google::protobuf::SetLogHandler(protobufLogHandler);
}

Labeless::~Labeless()
{
}


Labeless& Labeless::instance()
{
	static Labeless ll;
	return ll;
}

Settings Labeless::loadSettings()
{
	const bool nodeExists = storage::loadDbSettings(m_Settings);
	GlobalSettingsManger& gsm = GlobalSettingsManger::instance();

	do {
		if (!nodeExists)
		{
			m_Settings.host = gsm.value(GSK_PrevSelectedOllyHost, QString::fromStdString(m_Settings.host)).toString().toStdString();
			m_Settings.remoteModBase = get_imagebase();
		}

		m_Settings.analysePEHeader = gsm.value(GSK_AnalyzePEHeader, m_Settings.analysePEHeader).toBool();
		m_Settings.postProcessFixCallJumps = gsm.value(GSK_PostProcessFixCallJumps, m_Settings.postProcessFixCallJumps).toBool();
		m_Settings.overwriteWarning = static_cast<Settings::OverwriteWarning>(gsm.value(GSK_OverwriteWarning, m_Settings.overwriteWarning).toInt());
	} while (0);

	if (!nodeExists)
		storeSettings();

	return m_Settings;
}

void Labeless::storeSettings()
{
	storage::storeDbSettings(m_Settings);

	// global settings
	do {
		auto& gsm = GlobalSettingsManger::instance();
		gsm.setValue(GSK_PrevSelectedOllyHost, QString::fromStdString(m_Settings.host));
		gsm.setValue(GSK_AnalyzePEHeader, m_Settings.analysePEHeader);
		gsm.setValue(GSK_PostProcessFixCallJumps, m_Settings.postProcessFixCallJumps);
		gsm.setValue(GSK_OverwriteWarning, m_Settings.overwriteWarning);
	} while (0);

	PythonPaletteManager::instance().storeSettings();
}

bool Labeless::isUtf8StringValid(const char* const s, size_t len) const
{
	static const std::string kUTF8 = "UTF-8";
	QTextCodec* codec = QTextCodec::codecForName(kUTF8.c_str()); // TODO: may be cached?
	if (!codec)
		return false;

	QTextCodec::ConverterState state;
	codec->toUnicode(s, static_cast<int>(len), &state);

	return state.invalidChars == 0;
}

void Labeless::setTargetHostAddr(const std::string& ip, quint16 port)
{
	QMutexLocker lock(&m_ConfigLock);

	m_Settings.host = ip;
	m_Settings.port = port;
}

void Labeless::onSyncronizeAllRequested()
{
	if (!m_Settings.remoteModBase && QMessageBox::No == QMessageBox::question(util::ida::findIDAMainWindow(), tr("?"),
			tr("The \"Remote module base\" is zero.\nDo you want to continue?"),
			QMessageBox::Yes, QMessageBox::No))
		return;

	ScopedWaitBox waitBox("HIDECANCEL\nLabeless: synchronization in progress...");
	Q_UNUSED(waitBox);

	m_SynchronizeAllNow = true;
	msg("Labeless: do sync all now...\n");
	const size_t funcCnt = get_func_qty();

	LabelsSync::DataList labelPoints;
	CommentsSync::DataList commentPoints;

	qstring name;
	int flags = 0;
	if (m_Settings.localLabels)
		flags |= GN_LOCAL;
	if (m_Settings.demangle)
		flags |= GN_VISIBLE | GN_DEMANGLED | GN_SHORT;

	char segBuff[MAXSTR] = {};

	QHash<ea_t, std::string> ea2comment;

	auto fnJoinComment = [&ea2comment](ea_t ea, const std::string& cmt)
	{
		auto it = ea2comment.find(ea);
		if (it == ea2comment.end())
		{
			ea2comment[ea] = cmt;
			return;
		}
		std::string v = it.value();
		if (it.value() != cmt)
		{
			it.value() += ", " + cmt;
			//msg("LL: %#08x collision, multi-comment: %s\n", ea, it.value().c_str());
		}
	};


	// enumerate labels
	segment_t* s = static_cast<segment_t*>(segs.first_area_ptr());
	for (; s; s = static_cast<segment_t*>(segs.next_area_ptr(s->startEA)))
	{
		if (is_spec_segm(s->type))
			continue;
		const bool isExternSeg = (s->type & SEG_XTRN) == SEG_XTRN && get_segm_name(s, segBuff, MAXSTR) > 0 && std::string(segBuff) == NAME_EXTERN;
		if (isExternSeg)
			continue;
		ea_t ea = s->startEA;

		while (BADADDR != ea)
		{
			if (!has_dummy_name(getFlags(ea)) && get_true_name(&name, ea, flags) > 0 /*&& is_uname(name.c_str())*/)
			{
				std::string s = name.c_str();
				if (s.length() >= OLLY_TEXTLEN) // FIXME: move length checking to the backends
					s.erase(s.begin() + OLLY_TEXTLEN - 1, s.end());
				if (isUtf8StringValid(s.c_str(), s.length()))
				{
					if (util::ida::isFuncStart(ea))
					{
						// handle func
						if (m_Settings.commentsSync.testFlag(Settings::CS_FuncNameAsComment))
						{
							//msg("func as comment point: 0x%08" LL_FMT_EA_T " name: %s\n", ea, s.c_str());
							fnJoinComment(ea, s);
						}
						else
						{
							//msg("func point: 0x%08" LL_FMT_EA_T " name: %s\n", ea, s.c_str());
							labelPoints.push_back(LabelsSync::Data(ea, s));
						}
					}
					else
					{
						labelPoints.push_back(LabelsSync::Data(ea, s));
						//msg("label point: 0x%08" LL_FMT_EA_T " name: %s\n", ea, s.c_str());
					}
				}
			}

			ea = util::ida::getNextCodeOrDataEA(ea, m_Settings.nonCodeNames);
		}
	}


	if (!labelPoints.empty())
		addLabelsSyncData(labelPoints);

	const auto allLocalVars = m_Settings.commentsSync.testFlag(Settings::CS_LocalVarAll);
	if (m_Settings.commentsSync.testFlag(Settings::CS_LocalVar) || allLocalVars)
	{
		qstring memberName;
		strpath_t path;

		for (size_t i = 0; i < funcCnt; ++i)
		{
			func_t* const fn = getn_func(i);
			//struc_t* frame = get_frame(fn);

			func_item_iterator_t fnItemIt(fn);
			fnItemIt.first();
			while (fnItemIt.next_code())
			{
				const ea_t ea = fnItemIt.current();
				const flags_t flags = get_flags_novalue(ea);
				if (!isStkvar0(flags) && !isStkvar1(flags))
					continue;

				if (decode_insn(ea) <= 0)
					continue;

				for (size_t opNum = 0; opNum < 2; ++opNum)
				{
					sval_t v = 0;
					member_t* member = get_stkvar(::cmd.Operands[opNum], cmd.Operands[opNum].addr, &v);
					if (!member)
						continue;

					if (get_member_name2(&memberName, member->id) <= 0)
						continue;

					if (is_dummy_member_name(memberName.c_str()) && !allLocalVars)
						continue;

					if (memberName == kReturnAddrStackStructFieldName)
						continue;

					if (isStruct(member->flag))
					{
						adiff_t disp;
						qstring fields;
						path.len = get_struct_operand(ea, opNum, path.ids, &disp, &path.delta);
						if (path.len > 0 && (::cmd.itype != NN_lea || disp != 0))
						{
							append_struct_fields2(&fields, opNum, path.ids, path.len, byteflag(), &disp, path.delta, true);
							memberName += fields;
							fnJoinComment(ea, memberName.c_str());
							continue;
						}
					}

					if (allLocalVars)
						fnJoinComment(ea, memberName.c_str());
				}
			}
		}
	}

	if (m_Settings.commentsSync.testFlag(Settings::CS_IDAComment))
	{
		char buff[OLLY_TEXTLEN] = {};
		ssize_t len = 0;

		for (size_t i = 0; i < funcCnt; ++i)
		{
			func_t* fn = getn_func(i);
			if (!fn)
				continue;

			const char* nonRptCmt = get_func_cmt(fn, false);
			if (nonRptCmt && ::qstrlen(nonRptCmt))
				fnJoinComment(fn->startEA, nonRptCmt);

			const char* cmt = get_func_cmt(fn, true);
			if (!cmt || !::qstrlen(cmt))
				continue;

			if (!nonRptCmt)
				fnJoinComment(fn->startEA, cmt);

			const qlist<ea_t> refs = util::ida::codeRefsToCode(fn->startEA); // hlp::dataRefsToCode(fn->startEA);

			for (qlist<ea_t>::const_iterator it = refs.begin(), end = refs.end(); it != end; ++it)
			{
				fnJoinComment(*it, cmt);
			}
		}

		s = static_cast<segment_t*>(segs.first_area_ptr());
		for (; s; s = static_cast<segment_t*>(segs.next_area_ptr(s->startEA)))
		{
			for (ea_t ea = s->startEA, endEa = s->endEA; ea < endEa; ++ea)
			{
				if (!has_cmt(getFlags(ea)))
					continue;

				if ((len = get_cmt(ea, false, buff, OLLY_TEXTLEN)) > 0 &&
					isUtf8StringValid(buff, len))
				{
					fnJoinComment(ea, std::string(buff, static_cast<size_t>(len)));
					continue;
				}
				if ((len = get_cmt(ea, true, buff, OLLY_TEXTLEN)) > 0 &&
					isUtf8StringValid(buff, len))
				{
					fnJoinComment(ea, std::string(buff, static_cast<size_t>(len)));
				}
			}
		}
	}

	for (auto it = ea2comment.constBegin(), end = ea2comment.constEnd(); it != end; ++it)
		commentPoints.append(CommentsSync::Data(it.key(), it.value()));

	if (!commentPoints.isEmpty())
		addCommentsSyncData(commentPoints);

	msg("Labels: %d, comments: %d\n", labelPoints.count(), commentPoints.count());
	m_SynchronizeAllNow = false;

	//QRegExp reExtractFuncName("^[\\?]*([\\w\\d_]+)", Qt::CaseInsensitive);
	// TODO: handle option "remove func args"
}

bool Labeless::setEnabled()
{
	if (!!m_Enabled)
		return true;

	m_Enabled = true;
	enableRunScriptButton(true);

	return initialize();
}

void Labeless::enableMenuActions(bool enabled)
{
	for (int i = 0, e = m_MenuActions.length(); i < e; ++i)
	{
		const bool isLoadStubItem = m_MenuActions.at(i)->objectName() == kLabelessMenuLoadStubItemName ||
			m_MenuActions.at(i)->objectName() == kLabelessMenuLoadStubItemNameX64;
		m_MenuActions.at(i)->setEnabled(isLoadStubItem ? !enabled : enabled);
	}
}

std::string Labeless::ollyHost() const
{
	QMutexLocker lock(&m_ConfigLock);
	return m_Settings.host;
}

quint16 Labeless::ollyPort() const
{
	QMutexLocker lock(&m_ConfigLock);
	return m_Settings.port;
}

void Labeless::onRename(uint64_t ea, const std::string& newName)
{
	//if (m_LabelSyncOnRenameIfZero)
	//	return;
	if (!m_Settings.enabled)
		return;
	if (!m_DumpList.isEmpty() && m_DumpList.last().state != IDADump::ST_Done)
		return;
	msg("%s: rename addr %08llX to %s\n", __FUNCTION__, ea, newName.c_str());
	LabelsSync::DataList fncdl;
	fncdl.push_back(LabelsSync::Data(ea, newName));
	addLabelsSyncData(fncdl);
}

void Labeless::onAutoanalysisFinished()
{
	if (m_DumpList.isEmpty())
		return;

	IDADump& dump = m_DumpList.back();
	msg("%s: current state: %u\n", __FUNCTION__, dump.state);
	if (dump.state != IDADump::ST_PostAnalysis || !m_Settings.postProcessFixCallJumps)
	{
		dump.nextState(nullptr);
		return;
	}
	msg("%s: post processing calls/jumps\n", __FUNCTION__);
	autoWait();
	
	char disasm[MAXSTR] = {};

	const QRegExp reOpnd("\\s*(dword|near|far)\\s+ptr\\s+([\\w]+)\\+([a-f0-9]+)h?\\s*", Qt::CaseInsensitive);
	const QRegExp reShortOpnd("\\s*\\$\\+([a-f0-9]+)h?\\s*", Qt::CaseInsensitive);
	foreach(ReadMemoryRegions::t_memory m, dump.readMemRegions->data)
	{
		for (unsigned i = 0; i < m.size; ++i)
		{
			const ea_t ea = m.base + i;
			if (!isCode(get_flags_novalue(ea)))
				continue;

			if (decode_insn(ea) <= 0)
				continue;

			const ::insn_t c = ::cmd;
			const bool isJump = c.itype >= NN_ja && c.itype <= NN_jmpshort;
			const bool isCall = is_call_insn(ea); // c.itype >= NN_call && c.itype <= NN_callni;
			if (!isJump && !isCall)
				continue;
			const ea_t target = toEA(c.cs, c.Op1.addr);
			if (!::isEnabled(target))
				continue;

			if (!ua_outop2(ea, disasm, _countof(disasm), 0, 0) ||
				!tag_remove(disasm, disasm, MAXSTR))
			{
				continue;
			}
			const QString sDisasm = QString::fromLatin1(disasm);
			if (reOpnd.exactMatch(sDisasm))
			{
				const QString name = reOpnd.cap(2);
				if (is_uname(name.toStdString().c_str()))
					continue;
				msg("%s: found TODO for fix at: 0x%08" LL_FMT_EA_T ", opnd:%s\n", __FUNCTION__, ea, disasm);
				if (do_unknown(target, 0) && create_insn(target))
					msg("%s: ea: 0x%08" LL_FMT_EA_T " fixed\n", __FUNCTION__, ea);
			}
			else if (reShortOpnd.exactMatch(sDisasm))
			{
				msg("%s: found TODO for fix 2 at: 0x%08" LL_FMT_EA_T ", opnd: %s\n", __FUNCTION__, ea, disasm);
			}
		}
	}

	dump.nextState(nullptr);
}

bool Labeless::firstInit()
{
	if (QMainWindow* mw = util::ida::findIDAMainWindow())
	{
		if (QMenu* m = mw->menuBar()->addMenu("Labeless"))
		{
			m->setObjectName(kLabelessMenuObjectName);
#ifdef __X64__
			QMenu* loadStubMenu = m->addMenu(tr("Load stub database"));
			QAction* actLoadStub_x86 = loadStubMenu->addAction(tr("x86"), this, SLOT(onLoadStubDBRequested()));
			actLoadStub_x86->setObjectName(kLabelessMenuLoadStubItemName);
			QAction* actLoadStub_x64 = loadStubMenu->addAction(tr("x64"), this, SLOT(onLoadStubDBRequested()));
			actLoadStub_x64->setObjectName(kLabelessMenuLoadStubItemNameX64);
			m_MenuActions << actLoadStub_x86 << actLoadStub_x64;
#else // __X64__
			QAction* actLoadStub = m->addAction(tr("Load stub database..."), this, SLOT(onLoadStubDBRequested()));
			actLoadStub->setObjectName(kLabelessMenuLoadStubItemName);
			m_MenuActions << actLoadStub;
#endif // __X64__
			m->addSeparator();
			m_MenuActions << m->addAction(QIcon(":/run.png"), tr("Remote Python execution"), this, SLOT(onShowRemotePythonExecutionViewRequested()));
			QMenu* dumpMenu = m->addMenu(QIcon(":/dump.png"), tr("IDADump"));
			m_MenuActions << dumpMenu->addAction(tr("Wipe all and import..."), this, SLOT(onWipeAndImportRequested()));
			m_MenuActions << dumpMenu->addAction(tr("Keep existing and import..."), this, SLOT(onKeepAndImportRequested()));
			m_MenuActions << m->addAction(QIcon(":/sync.png"), tr("Sync labels now"), this, SLOT(onSyncronizeAllRequested()), Qt::ALT | Qt::SHIFT | Qt::Key_R);
			m->addSeparator();
			m_MenuActions << m->addAction(QIcon(":/settings.png"), tr("Settings..."), this, SLOT(onSettingsRequested()));
		}
	}
	static const action_desc_t sync_all_action = ACTION_DESC_LITERAL(
		kSyncAllNowActionName.c_str(),
		"Sync labels now",
		&g_SyncAllNowActionHandler,
		"Alt+Shift+R",
		NULL,
		-1);
	if (!register_action(sync_all_action))
		msg("%s: unable to register %s action\n", __FUNCTION__, kSyncAllNowActionName.c_str());

	enableMenuActions(false);
	return true;
}

bool Labeless::initialize()
{
	if (!m_Enabled || m_Initialized)
		return false;

	m_Settings = kDefaultSettings;
	loadSettings();
	loadImportTable();

	do {
		QMutexLocker lock(&m_ThreadLock);
		m_Thread = QPointer<QThread>(new QThread(this));
		RpcThreadWorker* worker = new RpcThreadWorker();
		CHECKED_CONNECT(connect(m_Thread.data(), SIGNAL(started()), worker, SLOT(main())));
		CHECKED_CONNECT(connect(worker, SIGNAL(destroyed()), m_Thread.data(), SLOT(quit()), Qt::QueuedConnection));
		//CHECKED_CONNECT(connect(m_Thread.data(), SIGNAL(finished()), worker, SLOT(deleteLater())));
		worker->moveToThread(m_Thread.data());
		m_Thread->start();
	} while (0);

	do {
		QMutexLocker lock(&m_AutoCompletionThreadLock);
		m_AutoCompletionThread = QPointer<QThread>(new QThread(this));
		JediCompletionWorker* worker = new JediCompletionWorker();
		CHECKED_CONNECT(connect(m_AutoCompletionThread.data(), SIGNAL(started()), worker, SLOT(main())));
		CHECKED_CONNECT(connect(worker, SIGNAL(destroyed()), m_AutoCompletionThread.data(), SLOT(quit()), Qt::QueuedConnection));
		//CHECKED_CONNECT(connect(m_AutoCompletionThread.data(), SIGNAL(finished()), worker, SLOT(deleteLater())));
		CHECKED_CONNECT(connect(worker, SIGNAL(completeFinished()), this, SLOT(onAutoCompletionFinished()), Qt::QueuedConnection));
		worker->moveToThread(m_AutoCompletionThread.data());
		m_AutoCompletionThread->start();
	} while (0);

	m_Initialized = true;

	return true;
}

void Labeless::terminate()
{
	static const unsigned kMaxThreadWaitTimeoutMsec = 20 * 1000;
	if (!m_Initialized)
		return;
	m_Initialized = false;

	if (m_PyOllyView && m_EditorTForm)
		m_PyOllyView->close();

	if (m_Enabled)
		m_Enabled = 0;

	unregister_action(kSyncAllNowActionName.c_str());

	do {
		QMutexLocker lock(&m_ThreadLock);
		if (m_Thread)
		{
			m_QueueCond.wakeAll();
			QEventLoop loop;
			while (m_Thread->isRunning())
			{
				loop.processEvents(QEventLoop::AllEvents, 500);
				m_Thread->wait(500);
			}
			m_Thread = nullptr;
		}
		lock.unlock();
		m_Queue.clear();
	} while (0);

	do {
		QMutexLocker lock(&m_AutoCompletionThreadLock);
		if (m_AutoCompletionThread)
		{
			m_AutoCompletionCond.wakeAll();
			QEventLoop loop;
			while (m_AutoCompletionThread->isRunning())
			{
				loop.processEvents(QEventLoop::AllEvents, 500);
				m_AutoCompletionThread->wait(500);
			}
			m_AutoCompletionThread = nullptr;
		}
		lock.unlock();
	} while (0);

	m_ExternSegData = ExternSegData();
}

void Labeless::shutdown()
{
	static bool shutdownCalled = false;
	if (shutdownCalled)
		return;
	shutdownCalled = true;
	terminate();
#ifdef __NT__
	WSACleanup();
#endif // __NT__
	GlobalSettingsManger::instance().detach();
}

void Labeless::addLabelsSyncData(const LabelsSync::DataList& sds)
{
	auto sync = std::make_shared<LabelsSync>();
	sync->data = sds;
	addRpcData(sync);
}

void Labeless::addCommentsSyncData(const CommentsSync::DataList& dl)
{
	auto sync = std::make_shared<CommentsSync>();
	sync->data = dl;
	addRpcData(sync);
}

RpcDataPtr Labeless::addRpcData(ICommandPtr cmd, RpcReadyToSendHandler ready, const QObject* receiver, const char* member, Qt::ConnectionType ct)
{
	RpcDataPtr rd(new RpcData());
	rd->iCmd = cmd;
	rd->readyToSendHandler = ready;

	if (!cmd->base)
		cmd->base = get_imagebase();
	if (!cmd->remoteBase)
		cmd->remoteBase = m_Settings.remoteModBase;
	if (!cmd->serialize(rd))
	{
		msg("%s: serialize() failed\n", __FUNCTION__);
		return RpcDataPtr();
	}
	return addRpcData(rd, receiver, member, ct);
}

RpcDataPtr Labeless::addRpcData(RpcDataPtr rpc, const QObject* receiver, const char* member, Qt::ConnectionType ct)
{
	if (receiver && member)
		CHECKED_CONNECT(connect(rpc.data(), SIGNAL(parsed()), receiver, member, ct));
	CHECKED_CONNECT(connect(rpc.data(), SIGNAL(received()), this, SLOT(onSyncResultReady()), Qt::QueuedConnection));
	CHECKED_CONNECT(connect(rpc.data(), SIGNAL(failed(QString)), this, SLOT(onRpcRequestFailed(QString)), Qt::QueuedConnection));
	m_QueueLock.lock();
	m_Queue.push_back(rpc);
	m_QueueLock.unlock();

	m_QueueCond.wakeOne();
	return rpc;
}

SOCKET Labeless::connectToHost(const std::string& host, uint16_t port, QString& errorMsg, bool keepAlive /*= true*/, quint32 recvtimeout /*= 30 * 60 * 1000*/)
{
	if (host.empty() || !port)
		return INVALID_SOCKET;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	addr.sin_addr.s_addr = inet_addr(host.c_str());
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		hostent* he = gethostbyname(host.c_str());
		if (!he)
		{
			errorMsg = QString("%1: Unable to resolve host: %2, err: %3")
				.arg(__FUNCTION__)
				.arg(QString::fromStdString(host))
				.arg(util::net::wsaErrorToString());
			return INVALID_SOCKET;
		}
		addr.sin_addr.s_addr = *(ulong*)he->h_addr_list[0];
	}
	if (addr.sin_addr.s_addr == INADDR_NONE || addr.sin_addr.s_addr == INADDR_ANY)
		return INVALID_SOCKET;

	SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		errorMsg = QString("%1: socket() failed. Error %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
		return s;
	}

	int bTrue = 1;
	bool failed = true;
	do
	{
		if (keepAlive)
		{
			if (SOCKET_ERROR == ::setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&bTrue), sizeof(bTrue)))
			{
				errorMsg = QString("%1: setsockopt(SO_KEEPALIVE) failed. LE: %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
				break;
			}
		}
		if (recvtimeout)
		{
#ifdef __NT__
			const quint32 recvTimeout = recvtimeout;
#elif defined(__unix__) || defined(__linux__)
			timeval recvTimeout;
			recvTimeout.tv_usec = 500000;
			recvTimeout.tv_sec = recvtimeout / 1000;
#endif // __NT__
			if (SOCKET_ERROR == ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&recvTimeout), sizeof(recvTimeout)))
			{
				errorMsg = QString("%1: setsockopt(SO_RCVTIMEO) failed. LE: %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
				break;
			}
		}
		if (keepAlive)
		{
#ifdef __NT__
			static const tcp_keepalive keepAliveCfg = { 1, 30 * 60 * 1000, 2000 };
			DWORD dwDummy = 0;
			if (SOCKET_ERROR == ::WSAIoctl(s, SIO_KEEPALIVE_VALS, LPVOID(&keepAliveCfg), sizeof(keepAliveCfg), nullptr, 0, &dwDummy, nullptr, nullptr))
			{
				errorMsg = QString("%1: WSAIoctl(SIO_KEEPALIVE_VALS) failed. LE: %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
				break;
			}
#elif defined(__unix__) || defined(__linux__)
			const quint32 dwKeepCnt = 10;
			const quint32 dwKeepInterval = 2;
			const quint32 dwKeepIdle = 30 * 60;
			if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, TCP_KEEPCNT, reinterpret_cast<const char*>(&dwKeepCnt), sizeof(dwKeepCnt)))
			{
				errorMsg = QString("%1: setsockopt(TCP_KEEPCNT) failed. LE: %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
				//break;
			}
			if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, TCP_KEEPINTVL, reinterpret_cast<const char*>(&dwKeepInterval), sizeof(dwKeepInterval)))
			{
				errorMsg = QString("%1: setsockopt(TCP_KEEPINTVL) failed. LE: %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
				//break;
			}
			if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, TCP_KEEPIDLE, reinterpret_cast<const char*>(&dwKeepIdle), sizeof(dwKeepIdle)))
			{
				errorMsg = QString("%1: setsockopt(TCP_KEEPIDLE) failed. LE: %2\n").arg(__FUNCTION__).arg(util::net::wsaErrorToString());
				//break;
			}
#endif
		}
		if (SOCKET_ERROR == ::connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
		{
			errorMsg = QString("%1: connect() to %2:%3 failed. Error: %4\n")
				.arg(__FUNCTION__)
				.arg(QString::fromStdString(host))
				.arg(port)
				.arg(util::net::wsaErrorToString());
			break;
		}
		failed = false;
	} while (0);
	if (failed)
	{
#ifdef __NT__
		closesocket(s);
#else
		close(s);
#endif
		s = INVALID_SOCKET;
	}
	return s;
}

void Labeless::onPyOllyFormClose()
{
	if (m_PyOllyView)
		m_PyOllyView->deleteLater();
	m_PyOllyView = nullptr;
	m_EditorTForm = nullptr;
}

void Labeless::onSettingsRequested()
{
	QMainWindow* idaw = util::ida::findIDAMainWindow();
	if (!idaw)
	{
		msg("%s: Unable to find IDA Main window\n", __FUNCTION__);
		return;
	}
	SettingsDialog sd(m_Settings, get_imagebase(), idaw);
	CHECKED_CONNECT(connect(&sd, SIGNAL(remotePythonExec()), this, SLOT(onShowRemotePythonExecutionViewRequested()), Qt::QueuedConnection));
	CHECKED_CONNECT(connect(&sd, SIGNAL(testConnection()), this, SLOT(onTestConnectRequested()), Qt::DirectConnection));
	CHECKED_CONNECT(connect(&sd, SIGNAL(syncronizeNow()), this, SLOT(onSyncronizeAllRequested()), Qt::QueuedConnection));

	if (QDialog::Accepted != sd.exec())
		return;
	sd.getSettings(m_Settings);
	if (sd.isPaletteChanged())
	{
		sd.getLightPalette(PythonPaletteManager::instance().lightPalette());
		sd.getDarkPalette(PythonPaletteManager::instance().darkPalette());
		if (m_PyOllyView)
			m_PyOllyView->onColorSchemeChanged();
	}
	storeSettings();
}

void Labeless::onLoadStubDBRequested()
{
	static const QString kStubX86Name = ":/stub.exe.bin";
	static const QString kStubX64Name = ":/stub64.exe.bin";
	static const QString kDatabaseExtX86 = ".idb";
	static const QString kDatabaseExtX64 = ".i64";

	bool isx86 = true;
	if (QAction* act = qobject_cast<QAction*>(sender()))
	{
		isx86 = act->objectName() == kLabelessMenuLoadStubItemName;
	}
	if (!::is_temp_database())
	{
		const auto rv = QMessageBox::question(util::ida::findIDAMainWindow(), tr("?"), tr("Do you really want to load sample DB?"),
				QMessageBox::Yes, QMessageBox::No);
		if (rv != QMessageBox::Yes)
			return;
	}
	const QString extension = isx86 ? kDatabaseExtX86 : kDatabaseExtX64;

	QString idbFileName = QFileDialog::getSaveFileName(util::ida::findIDAMainWindow(),
		tr("Select where to save new DB"),
		QString("sample%1").arg(extension),
		tr("IDA PRO IDB-file (*%1)").arg(extension));
	if (idbFileName.isEmpty())
		return;

	if (!idbFileName.endsWith(extension, Qt::CaseInsensitive))
		idbFileName.append(extension);
	const QString sampleFileName = idbFileName.left(idbFileName.length() - 4) + ".exe";

	const std::string sfn = sampleFileName.toStdString();

	QByteArray raw;
	do 
	{
		QFile fsrc(isx86 ? kStubX86Name : kStubX64Name);
		if (!fsrc.open(QIODevice::ReadOnly))
		{
			msg("%s: unable to open stub resource file\n", __FUNCTION__);
			return;
		}
		raw = fsrc.readAll();
		fsrc.close();
	} while (0);

	QFile f(sampleFileName);
	if (!f.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(util::ida::findIDAMainWindow(), tr("!"), tr("Unable to open file \"%1\" for write").arg(sampleFileName));
		return;
	}
	f.write(raw);
	f.close();
	term_database();

	const std::string appPath = qApp->applicationFilePath().toStdString();
	const char* args[2] = {};
	args[0] = appPath.c_str();
	args[1] = sfn.c_str();
	int v = 0;
	QStringList argv;
	init_database(2, args, &v);

	argv << QString::fromLatin1(::database_idb);
	term_database();
	QFile::remove(sampleFileName);

	QProcess::startDetached(qApp->applicationFilePath(), argv);
	shutdown();
	qApp->quit();
}

void Labeless::onShowRemotePythonExecutionViewRequested()
{
	if (m_EditorTForm)
		switchto_tform(m_EditorTForm, true);
	else
		openPythonEditorForm(FORM_TAB | FORM_MENU | FORM_RESTORE | FORM_QWIDGET | FORM_NOT_CLOSED_BY_ESC);
}

void Labeless::onRunScriptRequested()
{
	if (!m_Enabled || !m_PyOllyView)
		return;

	const std::string& idaScript = m_PyOllyView->getIDAScript().toStdString();
	std::string externObj;
	if (!idaScript.empty())
	{
		std::string errorMsg;
		if (!util::idapython::runScript(idaScript, externObj, errorMsg))
		{
			m_PyOllyView->prependStdoutLog(QString("runIDAPythonScript() failed with error: %1\n")
				.arg(QString::fromStdString(errorMsg)));
			return;
		}
	}

	const std::string& script = m_PyOllyView->getOllyScript().toStdString();
	if (script.empty())
		return;

	auto cmd = std::make_shared<ExecPyScript>();
	cmd->d.ollyScript = script;
	cmd->d.idaExtern = externObj;
	cmd->d.idaScript = idaScript;
	addRpcData(cmd, RpcReadyToSendHandler(), this, SLOT(onRunPythonScriptFinished()));
}

void Labeless::enableRunScriptButton(bool enabled)
{
	if (m_PyOllyView)
	{
		m_PyOllyView->enableRunScriptButton(enabled);
	}
}

void Labeless::setRemoteModuleBase(ea_t base)
{
	m_Settings.remoteModBase = base;
}

ea_t Labeless::remoteModuleBase()
{
	if (!m_Settings.remoteModBase)
		m_Settings.remoteModBase = get_imagebase();
	return m_Settings.remoteModBase;
}

void Labeless::onSyncResultReady()
{
	RpcDataPtr rd = qobject_cast<RpcData*>(sender());
	if (!rd)
		return;
	std::shared_ptr<void> rdGuard(nullptr, [rd](void*){ rd->deleteLater(); });

	if (!rd->iCmd)
	{
		msg("RpcData::iCmd is null\n");
		return;
	}
	const bool parsedOk = rd->iCmd->parseResponse(rd);

	do {
		if (!m_EditorTForm || !m_PyOllyView)
			break;

		LogItem logItem;
		logItem.jobId = rd->jobId;

		bool add = false;
		if (std::dynamic_pointer_cast<ExecPyScript>(rd->iCmd))
		{
			logItem.ollyScript = m_PyOllyView->getOllyScript();
			logItem.idaScript = m_PyOllyView->getIDAScript();
			add = true;
		}
		else if (Labeless::instance().isShowAllResponsesInLog())
		{
			logItem.ollyScript = QString::fromStdString(rd->script);
			add = true;
		}
		if (!parsedOk)
		{
			logItem.textStdErr += tr("parsing response failed\n");
			add = true;
		}

		if (!logItem.ollyScript.isEmpty() ||
			rd->response->has_error() ||
			!rd->response->std_out().empty() ||
			!rd->response->std_err().empty())
		{
			if (rd->response->has_error())
			{
				logItem.textStdErr += QString::fromStdString(rd->response->error()) + "\n";
			}
			logItem.textStdOut = QString::fromStdString(rd->response->std_out());
			logItem.textStdErr += QString::fromStdString(rd->response->std_err());
			add = true;
		}
		if (add)
		{
			addLogItem(logItem);
		}
	} while (0);

	if (!parsedOk)
	{
		msg("unable to parse response\n");
		return;
	}
	rd->emitParsed();
}

void Labeless::onRunPythonScriptFinished()
{
	RpcDataPtr pRD = qobject_cast<RpcData*>(sender());
	if (!pRD)
	{
		msg("%s: Invalid internal data structure (null)\n", __FUNCTION__);
		return;
	}
	auto req = std::dynamic_pointer_cast<ExecPyScript>(pRD->iCmd);
	if (!req)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}

	if (req->d.ollyResultIsSet)
	{
		std::string error;
		if (!util::idapython::setResultObject(req->d.ollyResult, error))
			msg("%s: setIDAPythonResultObject() failed with error: %s\n", __FUNCTION__, error.c_str());
	}
}

void Labeless::onGetBackendInfoFinished()
{
	RpcDataPtr pRD = qobject_cast<RpcData*>(sender());
	if (!pRD)
	{
		msg("%s: Invalid internal data structure (null)\n", __FUNCTION__);
		return;
	}
	auto req = std::dynamic_pointer_cast<GetBackendInfo>(pRD->iCmd);
	if (!req)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}
	const bool wipe = pRD->property("wipe").toBool();
	QString error;
#ifndef __X64__
	if (req->bitness != 32)
	{
		error = tr("You are running IDA PRO for 32-bit applications, but debug backend is %1-bit<br>"
			"Start idaq64.exe to work with non-32-bit applications.<br>")
			.arg(req->bitness);
	}
#endif // __X64__
	if (LABELESS_VER_STR != req->labeless_ver)
	{
		error += tr("The Labeless version mismatch IDA-side: %1, backend: %2<br>")
			.arg(QString::fromLatin1(LABELESS_VER_STR))
			.arg(QString::fromStdString(req->labeless_ver));
	}
	if ((::inf.is_64bit() && req->bitness != 64) ||
		(!::inf.is_64bit() && req->bitness != 32))
	{
		error += tr("Database bitness mismatch, IDA DB bitness: %1, remote app bitness: %2.<br>"
			"To dump remote application, you should load stub database.<br>"
			"Use menu [Labeless] -> [Load stub database] -> [x%3] and then try again.")
			.arg(::inf.is_64bit() ? 64: 32)
			.arg(req->bitness)
			.arg(req->bitness);
	}


	if (!error.isEmpty())
	{
		QMessageBox::information(util::ida::findIDAMainWindow(),
			tr("Bad backend"),
			error);
		return;
	}
	
	// continue working, load memory map
	auto getMemoryMapReq = std::make_shared<GetMemoryMapReq>();

	RpcDataPtr p = addRpcData(getMemoryMapReq, RpcReadyToSendHandler(), this, SLOT(onGetMemoryMapFinished()));
	p->setProperty("wipe", QVariant::fromValue(wipe));
}

bool Labeless::importCode(bool wipe)
{
	auto getBackendInfoReq = std::make_shared<GetBackendInfo>();
	RpcDataPtr p = addRpcData(getBackendInfoReq, RpcReadyToSendHandler(), this, SLOT(onGetBackendInfoFinished()));
	p->setProperty("wipe", QVariant::fromValue(wipe));
	
	return true;
}

void Labeless::onGetMemoryMapFinished()
{
	RpcDataPtr pRD = qobject_cast<RpcData*>(sender());
	if (!pRD)
	{
		msg("%s: Invalid internal data structure (null)\n", __FUNCTION__);
		return;
	}
	auto req = std::dynamic_pointer_cast<GetMemoryMapReq>(pRD->iCmd);
	if (!req)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}
	MemoryRegionList& vals = req->data;
	ChooseMemoryDialog cmd(vals, tr("Select memory to dump"), util::ida::findIDAMainWindow());
	if (QDialog::Accepted != cmd.exec())
		return; // canceled

	MemoryRegionList selected;
	if (!cmd.getSelectedMemory(selected) || selected.isEmpty())
		return;

	std::deque<segment_t*> existingSegs;
	const bool wipe = pRD->property("wipe").toBool();
	if (!wipe)
	{
		for (int i = 0, e = get_segm_qty(); i < e; ++i)
			existingSegs.push_back(getnseg(i));
	}

	msg("selected:\n");

	auto rmr = std::make_shared<ReadMemoryRegions>();
	ea_t base = selected.first().base;
	ea_t endEa = 0;
	for (int i = 0; i < selected.size(); ++i)
	{
		const auto& v = selected.at(i);
		if (v.base < base)
			base = v.base;
		if (v.base + v.size > endEa)
			endEa = v.base + v.size;
		msg("addr: %08llX, size: %08llX, protect: %08X\n", v.base, v.size, v.protect);
		rmr->data.push_back(ReadMemoryRegions::t_memory(v.base, v.size, v.protect, "", v.forceProtect));
	}
	if (!rmr->data.empty())
	{
		IDADump dump;
		dump.wipe = wipe;
		dump.analyzePEHeader = m_Settings.analysePEHeader;
		dump.readMemRegions = rmr;

		dump.checkPEHeaders = std::make_shared<CheckPEHeaders>();
		dump.checkPEHeaders->base = base;
		dump.checkPEHeaders->size = endEa - base;

		bool snapshotTaken = false;
		if (!wipe)
		{
			// check is overlaps
			area_t area(base, endEa);
			for (auto it = existingSegs.cbegin(), end = existingSegs.cend(); it != end; ++it)
			{
				segment_t* seg = *it;
				if (seg->overlaps(area))
				{
					if (!askForSnapshotBeforeOverwrite(&area, seg, snapshotTaken))
						return;
					if (snapshotTaken)
						break;
				}
			}
		}
		else
		{
			if (!askForSnapshotBeforeOverwrite(nullptr, nullptr, snapshotTaken))
				return;
		}

		m_DumpList.append(dump);

		RpcDataPtr p = m_DumpList.back().nextState(nullptr).state == IDADump::ST_CheckingPEHeader
			? addRpcData(dump.checkPEHeaders, RpcReadyToSendHandler(), this, SLOT(onCheckPEHeadersFinished()))
			: addRpcData(dump.readMemRegions, RpcReadyToSendHandler(), this, SLOT(onReadMemoryRegionsFinished()));
		p->setProperty("wipe", dump.wipe);
	}
}

bool Labeless::askForSnapshotBeforeOverwrite(const area_t* area, const segment_t* seg, bool& snapshotTaken)
{
	static const int kOverwrite = 1 << 0;
	static const int kTakeSnapAndOverwrite = 1 << 1;
	static const int kAlwaysTakeSnapAndOvrDontAsk = 1 << 2;
	static const int kAlwaysOvrDontAsk = 1 << 3;
	Q_UNUSED(kOverwrite);

	int chTakeSnapshot = 1;

	QString prefix;
	if (area && seg)
	{
		prefix = tr("One of selected regions[%1, %2) overlaps with existing segment[%3, %4).")
			.arg(area->startEA, 8, 16, QChar('0'))
			.arg(area->endEA, 8, 16, QChar('0'))
			.arg(seg->startEA, 8, 16, QChar('0'))
			.arg(seg->endEA, 8, 16, QChar('0'));
	}
	else
	{
		prefix = tr("You are going to overwrite data.");
	}

	if (Settings::OW_WarnAndCancel == m_Settings.overwriteWarning)
	{
		const QString msg = tr("%1\nExiting... The default behavior can be changed in Settings view.")
			.arg(prefix);
		QMessageBox::warning(util::ida::findIDAMainWindow(), tr("!"), msg);
		return false;
	}
	if (Settings::OW_AlwaysAsk == m_Settings.overwriteWarning)
	{
		const QString fmt = QString("?\n\n\n"
			"%1\n"
			"Do you want to overwrite/extend existing segment?"
			"\n"
			"<Overwrite :R>\n"
			"<Take IDB snapshot & overwrite :R>"
			"<Always take snapshot, don't ask :R>"
			"<Always overwrite, don't ask :R>>")
			.arg(prefix);

		if (!AskUsingForm_c(fmt.toStdString().c_str(), &chTakeSnapshot))
			return false;

		if (chTakeSnapshot & kAlwaysOvrDontAsk)
			GlobalSettingsManger::instance().setValue(GSK_OverwriteWarning, Settings::OW_Overwrite);
		else if (chTakeSnapshot & kAlwaysTakeSnapAndOvrDontAsk)
			GlobalSettingsManger::instance().setValue(GSK_OverwriteWarning, Settings::OW_TakeSnapshotAndOverwrite);
	}
	else if (Settings::OW_TakeSnapshotAndOverwrite == m_Settings.overwriteWarning)
	{
		chTakeSnapshot = kAlwaysTakeSnapAndOvrDontAsk;
	}

	if (!snapshotTaken && (chTakeSnapshot & (kTakeSnapAndOverwrite | kAlwaysTakeSnapAndOvrDontAsk)))
	{
		static const QString kSnapshotDescriptionFmt = "Labeless before dump - %1";
		qstring errmsg;
		snapshot_t snap;
		const QDateTime dt = QDateTime::currentDateTime();
		const QString dtStr = dt.toString("yyyy-MM-dd hh_mm_ss");
		const std::string snapshotName = kSnapshotDescriptionFmt.arg(dtStr).toStdString();

		::qstrncpy(snap.desc, snapshotName.c_str(), sizeof(snap.desc));
		snapshotTaken = take_database_snapshot(&snap, &errmsg);
		if (!snapshotTaken)
			msg("%s: Unable to take database snapshot, error: %s\n", __FUNCTION__, errmsg.c_str());
		else
			msg("%s: Database snapshot taken (%s)\n", __FUNCTION__, snap.filename);
	}
	return true;
}

void Labeless::onCheckPEHeadersFinished()
{
	RpcDataPtr pRD = qobject_cast<RpcData*>(sender());
	if (!pRD)
	{
		msg("%s: Invalid internal data structure (null)\n", __FUNCTION__);
		return;
	}
	auto req = std::dynamic_pointer_cast<CheckPEHeaders>(pRD->iCmd);
	if (!req)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}

	if (m_DumpList.isEmpty())
	{
		msg("%s: internal data is wrong, no ImportCodeInfo found\n", __FUNCTION__);
		return;
	}
	IDADump& info = m_DumpList.back();
	info.exports = req->exports;
	info.sections = req->sections;
	for (int i = 0; i < info.sections.size(); ++i)
	{
		info.sections[i].va += req->base;
	}
	info.nextState(pRD).nextState(nullptr);
	RpcDataPtr p = addRpcData(info.readMemRegions, RpcReadyToSendHandler(), this, SLOT(onReadMemoryRegionsFinished()));
	p->setProperty("wipe", info.wipe);
}

void Labeless::onReadMemoryRegionsFinished()
{
	RpcDataPtr pRD = qobject_cast<RpcData*>(sender());
	if (!pRD)
	{
		msg("%s: invalid sender\n", __FUNCTION__);
		return;
	}
	auto rmr = std::dynamic_pointer_cast<ReadMemoryRegions>(pRD->iCmd);
	if (!rmr)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}
	if (rmr->data.empty())
	{
		msg("%s: empty memory regions", __FUNCTION__);
		return;
	}

	if (m_DumpList.isEmpty())
	{
		msg("%s: internal data is wrong, no ImportCodeInfo found\n", __FUNCTION__);
		return;
	}
	IDADump& icInfo = m_DumpList.back();
	icInfo.nextState(pRD);

	if (pRD->property("wipe").toBool())
	{
		while (segment_t* seg = getnseg(0))
			del_segm(seg->startEA, SEGMOD_KILL);

		m_ExternSegData = ExternSegData();
		if (!rmr->data.empty())
		{
			const ea_t newBase = rmr->data.front().base;
			set_imagebase(newBase);
			msg("ImageBase automatically changed to %08" LL_FMT_EA_T " because 'wipe' is requested\n", newBase);
			m_Settings.remoteModBase = newBase;
			storeSettings();
		}
	}

	m_LabelSyncOnRenameIfZero = rmr->data.size();

	const ea_t region_base = rmr->data.front().base;
	const uint64_t region_size = rmr->data.back().base + rmr->data.back().size - region_base;

	icInfo.nextState(nullptr);

	for (int i = 0; i < rmr->data.size(); ++i)
	{
		const ReadMemoryRegions::t_memory& m = rmr->data.at(i);
		if (m.raw.size() != m.size)
		{
			msg("%s: Raw data size mismatch (expected %08llX, received %08" MY_PRIXPTR ")\n", __FUNCTION__, m.size, m.raw.size());
			return;
		}
		if (!mergeMemoryRegion(icInfo, m, region_base, region_size))
			return;

		auto gdp = std::make_shared<AnalyzeExternalRefs>();
		gdp->req.eaFrom = m.base;
		gdp->req.eaTo = m.base + m.size;
		gdp->req.increment = 1;
		gdp->req.base = region_base;
		gdp->req.size = region_size;

		auto p = addRpcData(gdp, RpcReadyToSendHandler(), this, SLOT(onAnalyzeExternalRefsFinished()));
		p->setProperty("wipe", icInfo.wipe);

		icInfo.analyzeExtRefs.append(IDADump::AnalyseExtRefsWrapper(p));
	}
}

void Labeless::getRegionPermissionsAndType(const IDADump& icInfo, const ReadMemoryRegions::t_memory& m, uchar& perm, uchar& type) const
{
	perm = type = 0;

	area_t area(m.base, m.base + m.size);
	auto sectionIt = std::find_if(icInfo.sections.constBegin(), icInfo.sections.constEnd(), [area](const Section& s) -> bool {
		return area.overlaps(area_t(s.va, s.va + s.va_size));
		/*const int diff1 = static_cast<int>(m.base + m.size) - static_cast<int>(s.va);
		const int diff2 = static_cast<int>(s.va + s.va_size) - static_cast<int>(m.base);
		const int maxIntersectionSize = s.va_size + m.size;
		bool rv = (diff1 > 0 && diff1 < maxIntersectionSize) ||
			(diff2 > 0 && diff2 < maxIntersectionSize);
		return rv;*/
	});

	const auto sProtect = util::ida::decodeMemoryProtect(m.protect);

	if (sectionIt != icInfo.sections.constEnd())
	{
		msg("Section found for region{ ea: %08" LL_FMT_EA_T ", size: %08" LL_FMT_EA_T " } is section { name: %s, va: %08llX, size: %08llX, ch: %08X }\n",
			area.startEA, area.size(), sectionIt->name.c_str(), sectionIt->va, sectionIt->va_size, sectionIt->characteristics);

		const uint32_t ch = sectionIt->characteristics;
		type = (ch & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE))
			? SEG_CODE
			: SEG_DATA;

		if ((ch & IMAGE_SCN_MEM_EXECUTE) ||
			(m.forceProtect && (sProtect & util::ida::MPF_EXEC)))
			perm |= SEGPERM_EXEC;

		if ((ch & IMAGE_SCN_MEM_WRITE) ||
			(m.forceProtect && (sProtect & util::ida::MPF_WRITE)))
			perm |= SEGPERM_WRITE;

		if ((ch & IMAGE_SCN_MEM_READ) ||
			(m.forceProtect && (sProtect & util::ida::MPF_READ)))
			perm |= SEGPERM_READ;

		return;
	}
	
	if (sProtect & util::ida::MPF_EXEC)
		type = SEG_CODE;
	else
		type = SEG_DATA;

	if (sProtect & util::ida::MPF_EXEC)
		perm |= SEGPERM_EXEC;
	if (sProtect & util::ida::MPF_WRITE)
		perm |= SEGPERM_WRITE;
	if (sProtect & util::ida::MPF_READ)
		perm |= SEGPERM_READ;
}

bool Labeless::mergeMemoryRegion(IDADump& icInfo, const ReadMemoryRegions::t_memory& m, ea_t region_base, uint64_t region_size)
{
	Q_UNUSED(region_base);
	Q_UNUSED(region_size);

	uchar perm = 0;
	uchar type = 0;
	const area_t area(m.base, m.base + m.size);

	getRegionPermissionsAndType(icInfo, m, perm, type);

	segment_t seg;
	if (!createSegment(area, perm, type, m.raw, seg))
	{
		msg("%s: createSegment() failed\n", __FUNCTION__);
		return false;
	}
	return true;
}

segment_t* Labeless::getFirstOverlappedSegment(const area_t& area, segment_t* exceptThisSegment)
{
	segment_t* rv = getseg(area.startEA);
	if (rv && (!exceptThisSegment || exceptThisSegment != rv))
		return rv;
	while (rv)
	{
		rv = get_next_seg(rv->startEA);
		if (rv && rv->overlaps(area) && (!exceptThisSegment || rv != exceptThisSegment))
			return rv;
	}
	return nullptr;
}

bool Labeless::createSegment(const area_t& area, uchar perm, uchar type, const std::string& data, segment_t& result)
{
	result = m_CreatedSegments.push_back();
	memset(&result, 0, sizeof(result));
	static_cast<area_t&>(result) = area;

	result.bitness = ::inf.is_64bit() ? 2 : 1;
	result.sel = setup_selector(0);
	result.perm = perm;
	result.type = type;

	result.comb = scPub;
	result.align = saRelByte;
	result.color = DEFCOLOR;
	result.set_visible_segm(true);
	result.update();

	qstring name;
	name.sprnt("SEG%03u", m_CreatedSegments.size() - 1);
	const bool ok = add_segm_ex(&result, name.c_str(), type == SEG_CODE ? "CODE" : "DATA", ADDSEG_QUIET | ADDSEG_NOSREG);
	if (!ok)
	{
		info("Unable to create \"%s\" segment\n", name.c_str());
		return false;
	}
	if (!set_default_segreg_value(getseg(area.startEA), str2reg("es"), 0))
		msg("%s: set_default_segreg_value('es') failed\n", __FUNCTION__);
	if (!set_default_segreg_value(getseg(area.startEA), str2reg("ds"), 0))
		msg("%s: set_default_segreg_value('ds') failed\n", __FUNCTION__);
	if (!set_default_segreg_value(getseg(area.startEA), str2reg("ss"), 0))
		msg("%s: set_default_segreg_value('ss') failed\n", __FUNCTION__);

	mem2base(data.c_str(), area.startEA, area.endEA, -1);

	do_unknown_range(area.startEA, area.size(), DOUNK_EXPAND);
	noUsed(area.startEA, area.endEA); // plan to reanalyze
	return true;
}

void Labeless::onAnalyzeExternalRefsFinished()
{
	RpcDataPtr rd = qobject_cast<RpcData*>(sender());
	if (!rd)
		return;
	auto gdp = std::dynamic_pointer_cast<AnalyzeExternalRefs>(rd->iCmd);
	if (!gdp)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}
	if (gdp->pending && gdp->jobId && gdp->error.empty())
	{
		if (rd->retryCount++)
		{
			const QDateTime now = QDateTime::currentDateTime();
			// add delayed for 0.5 minute task
			rd->readyToSendHandler = [now](RpcData*) { return now.secsTo(QDateTime::currentDateTime()) >= 30; };
		}
		addRpcData(rd);
		return;
	}
	msg("%s", QString("%1: job id: %2 %3\n")
			.arg(__FUNCTION__)
			.arg(rd->jobId)
			.arg(gdp->error.empty() ? "ok" : gdp->error.c_str()).toStdString().c_str());
	if (!gdp->error.empty())
		return;

	if (m_DumpList.isEmpty())
	{
		msg("%s: internal data is wrong, no IDADump item found\n", __FUNCTION__);
		return;
	}
	IDADump& icInfo = m_DumpList.back();

	const unsigned targetPtrSize = ::inf.is_64bit() ? sizeof(uint64_t) : sizeof(uint32_t);

	if (gdp->rip >= gdp->req.eaFrom && gdp->rip <= gdp->req.eaTo - targetPtrSize)
	{
		const auto& entryName = getNewNameOfEntry();
		add_entry(gdp->rip, gdp->rip, entryName.c_str(), true);
		msg("Entry created %s, waiting for finish auto-analysis\n", entryName.c_str());
		autoWait();
	}
	std::set<uint64_t> exportEntries;
	for (int i = 0, e = icInfo.exports.size(); i < e; ++i)
	{
		auto ea = icInfo.exports.at(i).ea;
		if (ea >= gdp->req.base && ea <= gdp->req.eaTo)
		{
			exportEntries.insert(ea);
			add_entry(ea, ea, icInfo.exports.at(i).name.c_str(), true);
		}
	}
	std::map<uint64_t, AnalyzeExternalRefs::PointerData> constPlaces;
	for (int i = 0; i < gdp->ptrs.size(); ++i)
		constPlaces[gdp->ptrs.at(i).ea] = gdp->ptrs.at(i);

	if (!createImportSegments(constPlaces))
	{
		msg("%s: createImportSegments() failed\n", __FUNCTION__);
		// TODO
	}

	char buff[MAXSTR] = {};
	for (int i = 0; i < gdp->rdl.size(); ++i)
	{
		const AnalyzeExternalRefs::RefData& rd = gdp->rdl.at(i);
		const ea_t ea = rd.instrEA;
		if (!isCode(get_flags_novalue(ea)))
		{
			const auto autoLen = create_insn(ea);
			if (autoLen != static_cast<int>(rd.len))
				continue;
		}
		if (get_item_end(ea) - ea != rd.len)
			continue;
		std::string mnem = rd.dis;
		const auto p = mnem.find(' ');
		if (p == mnem.npos)
			continue;
		mnem.erase(p, mnem.size() - p);
		qstrupr(&mnem[0]);

		if (!ua_mnem(ea, buff, MAXSTR) || !::qstrlen(buff) || !qstrupr(buff) || std::string(buff).substr(0, 1) != mnem.substr(0, 1))
			continue;

		const auto val = rd.val;
		int opndNum = -1;
		insn_t c = ::cmd;
		for (int e = 0; e < UA_MAXOP && opndNum == -1; ++e)
			if (c.Operands[e].addr == val || c.Operands[e].value == val)
				opndNum = e;
		if (opndNum == -1)
			continue;

		qstring newName;
		newName.sprnt("EXTERN_%s_%s", rd.module.c_str(), rd.proc.c_str());
		set_forced_operand(ea, opndNum, newName.c_str());
	}
	//m_LabelSyncOnRenameIfZero -= 1;

	updateImportsNode();
	request_refresh(IWID_IMPORTS);
	for (int i = 0; i < get_segm_qty(); ++i)
	{
		segment_t* seg = getnseg(i);
		auto_mark_range(seg->startEA, seg->endEA, AU_USED);
	}

	open_imports_window(0);
	if (icInfo.nextState(rd).state == IDADump::ST_PostAnalysis)
	{
		onAutoanalysisFinished();
	}
}

void Labeless::onAutoCompleteRemoteFinished()
{
	RpcDataPtr pRD = qobject_cast<RpcData*>(sender());
	if (!pRD)
	{
		msg("%s: invalid sender\n", __FUNCTION__);
		return;
	}
	auto acc = std::dynamic_pointer_cast<AutoCompleteCode>(pRD->iCmd);
	if (!acc)
	{
		msg("%s: Invalid type of ICommand\n", __FUNCTION__);
		return;
	}

	auto r = pRD->property("r").value<QSharedPointer<jedi::Request>>();
	if (!r)
	{
		msg("%s: jedi::Request is null\n", __FUNCTION__);
		return;
	}
	
	if (!QMetaObject::invokeMethod(r->rcv, "onAutoCompleteFinished",
		Qt::QueuedConnection,
		Q_ARG(QSharedPointer<jedi::Result>, acc->jresult)))
	{
		msg("%s: QMetaObject::invokeMethod() failed\n", __FUNCTION__);
		return;
	}
}

void Labeless::addAPIConst(const AnalyzeExternalRefs::PointerData& pd)
{
	const auto ptrSize = targetPtrSize();

	do_unknown_range(pd.ea, ptrSize, DOUNK_DELNAMES);
	if (!make_dword_ptr(pd.ea, ptrSize))
		msg("%s: make_dword_ptr() failed for ea: %08llX\n", __FUNCTION__, pd.ea);

	if (!do_name_anyway(pd.ea, pd.procName.c_str()))
	{
		msg("%s: do_name_anyway() failed for ea: %08llX\n", __FUNCTION__, pd.ea);
		set_cmt(pd.ea, pd.procName.c_str(), false);
	}

	// add enum value
	const std::string enumValueName = pd.module + "_" + pd.procName;
	bool enumValExists = true;

	if (BADNODE == get_enum_member_by_name(enumValueName.c_str()))
	{
		const uval_t val = ::inf.is_64bit() ? get_qword(pd.ea) : get_long(pd.ea);
		enumValExists = addAPIEnumValue(enumValueName, val);
		if (!enumValExists)
			msg("%s: addAPIEnumValue() failed for ea: %08llX\n", __FUNCTION__, pd.ea);
	}

	// set name of old entry to be pointed to our APIs enum
	if (enumValExists && !op_enum(pd.ea, 0, get_enum(kAPIEnumName.c_str()), 0))
		msg("%s: op_enum() failed for ea: %08llX\n", __FUNCTION__, pd.ea);
}

void Labeless::openPythonEditorForm(int options)
{
	HWND hwnd = NULL;
	m_EditorTForm = create_tform("PyOlly", &hwnd);
	open_tform(m_EditorTForm, options);
}

void Labeless::onTestConnectRequested()
{
	SettingsDialog* sd = qobject_cast<SettingsDialog*>(sender());
	if (!sd)
		return;
	Settings tmpSettings;
	sd->getSettings(tmpSettings);
	auto host = tmpSettings.host;
	auto port = tmpSettings.port;

	show_wait_box("HIDECANCEL\nChecking the connection to %s:%hu...", host.c_str(), port);

	PingThread* const pt = new PingThread(host, port);
	connect(pt, SIGNAL(finished()), pt, SLOT(deleteLater()));
	pt->start();
}

void Labeless::onTestConnectFinished(bool ok, const QString& error)
{
	hide_wait_box();
	if (ok)
		QMessageBox::information(util::ida::findIDAMainWindow(), kLabelessTitle, tr("Successfully connected!"));
	else
		QMessageBox::warning(util::ida::findIDAMainWindow(), kLabelessTitle, tr("Test failed, error:\n%1").arg(error));
}

void Labeless::onAutoCompletionFinished()
{
	QMutexLocker lock(&m_AutoCompletionLock);
	if (m_AutoCompletionRequest->rcv && m_AutoCompletionResult)
	{
		if (!QMetaObject::invokeMethod(m_AutoCompletionRequest->rcv, "onAutoCompleteFinished",
			Qt::QueuedConnection,
			Q_ARG(QSharedPointer<jedi::Result>, m_AutoCompletionResult)))
		{
			msg("%s: QMetaObject::invokeMethod() failed\n", __FUNCTION__);
			return;
		}
	}
	m_AutoCompletionRequest.clear();
	m_AutoCompletionResult.clear();
	m_AutoCompletionState->state = jedi::State::RS_DONE;
}

void Labeless::onAutoCompleteRequested(QSharedPointer<jedi::Request> r)
{
	QMutexLocker lock(&m_AutoCompletionLock);
	if (m_AutoCompletionState->state == jedi::State::RS_DONE) 
	{
		// means that user should switch state from RS_FINISHED to RS_DONE to accent new requests
		m_AutoCompletionRequest = r;
		m_AutoCompletionRequest->rcv = sender();
		m_AutoCompletionState->state = jedi::State::RS_ASKED;
	}
	lock.unlock();
	m_AutoCompletionCond.wakeOne();
}

void Labeless::onAutoCompleteRemoteRequested(QSharedPointer<jedi::Request> r)
{
	// TODO: 
	auto cmd = std::make_shared<AutoCompleteCode>();
	cmd->source = r->script.toUtf8();
	cmd->zline = r->zline;
	cmd->zcol = r->zcol;
	r->rcv = sender();
	cmd->callSigsOnly = false; // FIXME
	RpcDataPtr p = addRpcData(cmd, RpcReadyToSendHandler(), this, SLOT(onAutoCompleteRemoteFinished()));
	p->setProperty("r", QVariant::fromValue(r));
}

bool Labeless::testConnect(const std::string& host, uint16_t port, QString& errorMsg)
{
	errorMsg.clear();
	SOCKET s = connectToHost(host, port, errorMsg, false, 0);
	if (INVALID_SOCKET == s)
	{
		if (errorMsg.isEmpty())
			errorMsg = "connectToHost() failed\n";
		return false;
	}
	std::shared_ptr<void> guard (nullptr, [s](void*){
#ifdef __NT__
		closesocket(s);
#else
		close(s);
#endif
	}); // clean-up guard
	Q_UNUSED(guard);

	rpc::Execute command;
	command.set_script(
		"import sys\n"
		"from labeless.py_olly import labeless_ver\n"
		"print 'pong'\n"
		"print >> sys.stderr, 'v:%s' % labeless_ver()");

	const std::string& message = command.SerializeAsString();
	const uint64_t messageLen = static_cast<uint64_t>(message.length());

	if (!util::net::sockSendBuff(s, reinterpret_cast<const char*>(&messageLen), sizeof(messageLen)))
		return false;

	if (!util::net::sockSendString(s, message))
	{
		errorMsg = "sockSendString() failed";
		return false;
	}

	std::string rawResponse;
	if (!util::net::sockRecvAll(s, rawResponse) || rawResponse.empty())
	{
		errorMsg = QString("sockRecvAll() failed, error: %1").arg(util::net::wsaErrorToString());
		return false;
	}

	rpc::Response response;

	if (!response.ParseFromString(rawResponse))
	{
		errorMsg = "ParseFromString() failed";
		return false;
	}
	const std::string out = response.std_out();
	const std::string err = response.std_err();
	const bool rv = out.length() >= 4 && out.substr(0, 4) == "pong";
	if (!rv)
		errorMsg = "remote error";
	static const std::string kVer = "v:" LABELESS_VER_STR;

	if (err.length() < kVer.length())
		errorMsg = QString::fromStdString("Invalid version response: " + err);
	else if (err.substr(0, kVer.length()) != kVer)
		errorMsg = QString::fromStdString("version mismatch. Labeless IDA: " + kVer + ". But Labeless in the debugger: " + err);
	
	return rv && errorMsg.isEmpty();
}

size_t Labeless::loadImportTable()
{
	m_ExternSegData = ExternSegData();
	storage::loadExternSegData(m_ExternSegData);

	return m_ExternSegData.imports.size();
}

void Labeless::storeImportTable()
{
	if (!storage::storeExternSegData(m_ExternSegData))
		msg("%s: storage::storeExternSegData() failed\n", __FUNCTION__);
}

bool Labeless::createImportSegments(const std::map<uint64_t, AnalyzeExternalRefs::PointerData>& impData)
{
	static const int kIDASDKBitness32 = 1;
	static const int kIDASDKBitness64 = 2;

	if (impData.empty())
		return true;

	char segName[MAXSTR] = {};
	bool ok = true;

	const auto ptrSize = targetPtrSize();
	for (auto it = impData.cbegin(), end = impData.cend(); it != end; ++it)
	{
		const uint64_t ea = it->first;
		const auto& pd = it->second;

		segment_t* s = nullptr;
		do
		{
			if (get_segm_name(ea - ptrSize, segName, _countof(segName)) > 0 &&
				std::string(segName) == NAME_EXTERN &&
				(s = getseg(ea - ptrSize)))
			{
				if (segment_t* s2 = getseg(ea))
				{
					if (s2->startEA == ea)
					{
						if (!move_segm_start(ea, ea + ptrSize, -2))
						{
							msg("%s: move_segm_start(0x%08" LL_FMT_EA_T ", 0x%08" LL_FMT_EA_T ") failed\n", __FUNCTION__, s->startEA, static_cast<ea_t>(ea + ptrSize));
						}
						else
						{
							if (!set_segm_end(s->startEA, ea + ptrSize, SEGMOD_KEEP | SEGMOD_SILENT))
								msg("%s: set_segm_end(0x%08" LL_FMT_EA_T ", 0x%08" LL_FMT_EA_T ") failed\n", __FUNCTION__, s->startEA, static_cast<ea_t>(ea + ptrSize));
							//else
							//	msg("Import segment [0x%08" LL_FMT_EA_T ";0x%08" LL_FMT_EA_T ") was grew up at end\n", s->startEA, s->endEA);
						}
					}
				}
				break;
			}

			if (get_segm_name(ea + ptrSize, segName, _countof(segName)) > 0 &&
				std::string(segName) == NAME_EXTERN &&
				(s = getseg(ea + ptrSize)))
			{
				if (!move_segm_start(s->startEA, ea, -2))
					msg("%s: move_segm_start(0x%08" LL_FMT_EA_T ", 0x%08" LL_FMT_EA_T ") failed\n", __FUNCTION__, s->startEA, static_cast<ea_t>(ea + ptrSize));
				//else
				//	msg("Import segment [0x%08" LL_FMT_EA_T ";0x%08" LL_FMT_EA_T ") was grew up at start", s->startEA, s->endEA);
				break;
			}

			// create new segment
			segment_t ns;
			s = getseg(ea);
			if (s != NULL)
				ns.sel = s->sel;
			else
				ns.sel = setup_selector(0);

			ns.startEA = ea;
			ns.endEA = ea + ptrSize;
			ns.type = SEG_XTRN;
			ns.perm = SEGPERM_READ;
			ns.comb = scPub;
			ns.align = saRelPara;
			ns.color = DEFCOLOR;
			if (::inf.is_64bit())
				ns.bitness = kIDASDKBitness64;
			else
				ns.bitness = kIDASDKBitness32;

			ns.set_visible_segm(true);
			ns.set_loader_segm(true);
			ok &= add_segm_ex(&ns, NAME_EXTERN, "XTRN", ADDSEG_NOSREG) != 0;
			if (ok)
				msg("Import segment [0x%08" LL_FMT_EA_T ";0x%08" LL_FMT_EA_T ") was created successfully\n", ns.startEA, ns.endEA);
		} while (0);

		addAPIConst(it->second);

		uint32_t ordinalVal = 0;
		if (!pd.procName.empty() && pd.procName[0] == '#')
			ordinalVal = atol(pd.procName.substr(1).c_str());

		m_ExternSegData.addAPI(pd.module, pd.procName, pd.ea, ordinalVal);
		const std::string joined = pd.module + "." + pd.procName;
		m_ExternRefsMap[pd.ea] = joined;
	}

	return true;
}

void Labeless::updateImportsNode()
{
	std::unordered_map<std::string, nodeidx_t> module2Index;
	std::set<std::string> existingAPIs;

	char modname[MAXSTR + 4] = {};
	char funcName[MAXSTR];
	uint64_t modulesCount = 0;
	for (nodeidx_t idx = import_node.alt1st(); idx != BADNODE; idx = import_node.altnxt(idx))
	{
		modulesCount++;
		if (import_node.supstr(idx, modname, sizeof(modname)) <= 0)
			continue;

		std::string modName = modname;
		netnode modNode = import_node.altval(idx);
		module2Index[modName] = idx;

		for (uval_t ord = modNode.alt1st(); ord != BADNODE; ord = modNode.altnxt(ord))
		{
			ea_t ea = modNode.altval(ord);
			if (modNode.supstr(ea, funcName, sizeof(funcName)) > 0 && getFlags(ea))
				existingAPIs.insert(funcName);
		}

		for (ea_t ea = modNode.sup1st(); ea != BADADDR; ea = modNode.supnxt(ea))
		{
			if (modNode.supstr(ea, funcName, sizeof(funcName)) > 0 && getFlags(ea))
				existingAPIs.insert(funcName);
		}
	}

	auto it = m_ExternSegData.imports.cbegin();
	for (; it != m_ExternSegData.imports.cend(); ++it)
	{
		const ImportEntry& ie = it->second;
		if (existingAPIs.count(ie.proc))
			continue;
		netnode nModule;
		const bool moduleExists = module2Index.find(ie.module) != module2Index.end();
		if (moduleExists)
		{
			nModule = import_node.altval(module2Index[ie.module]);
		}
		else
		{
			qstring s;
			s.sprnt("$lib %s", ie.module.c_str());
			nModule.create(s.c_str());
		}

		nModule.supset(ie.ea, ie.proc.c_str());

		if (ie.ordinal)
		{
			nModule.altset(ie.ordinal, ie.ea); // FIXME

			set_cmt(ie.ea, (ie.module + "." + ie.proc).c_str(), true);
		}
		if (!moduleExists)
		{
			nodeidx_t lastFreeImpNode = import_node.altval(-1);
			import_node.altset(-1, lastFreeImpNode + 1);
			import_node.altset(lastFreeImpNode, nModule);
			import_node.supset(lastFreeImpNode, ie.module.c_str());
			import_node.supstr(lastFreeImpNode, modname, sizeof(modname));
			module2Index[ie.module] = lastFreeImpNode;
			modulesCount++;

			import_module(ie.module.c_str(), nullptr, nModule, nullptr, "win");
			import_node.supstr(lastFreeImpNode, modname, sizeof(modname));
		}
		existingAPIs.insert(ie.proc);
	}
	storeImportTable();
	msg("import mods after update: %u\n", import_node.altval(-1));
}

qstring Labeless::getNewNameOfEntry() const
{
	const size_t count = get_entry_qty();
	qstring rv;
	char name[2048] = {};
	std::set<std::string> existing;
	for (size_t i = 0; i < count; ++i)
		if (get_entry_name(get_entry_ordinal(i), name, 2048))
			existing.insert(std::string(name));
	for (size_t i = 0;; ++i)
	{
		rv.sprnt("start_%03u", i);
		if (existing.find(rv.c_str()) == existing.end())
			return rv;
	}
	return qstring();
}

bool Labeless::addAPIEnumValue(const std::string& name, uval_t value)
{
	begin_type_updating(UTP_ENUM);

	enum_t id = get_enum(kAPIEnumName.c_str());
	if (id == BADNODE)
	{
		id = add_enum(-1, kAPIEnumName.c_str(), DEFMASK);
		if (id == BADNODE)
			return false;
		set_enum_bf(id, false);
		set_enum_hidden(id, true);
	}
	add_enum_member(id, ("OAEC_" + name).c_str(), value);
	end_type_updating(UTP_ENUM);
	return true;
}

void Labeless::onMakeCode(ea_t ea, ::asize_t size)
{
	Q_UNUSED(ea);
	Q_UNUSED(size);
	if (m_IgnoreMakeCode)
		return;
	/*char disasm[MAXSTR] = {};
	qstring dis;
	if (generate_disasm_line(ea, disasm, MAXSTR, GENDSM_FORCE_CODE) &&
		tag_remove(disasm, disasm, MAXSTR))
		dis = disasm;*/
	//msg("on make_code: ea: %08" LL_FMT_EA_T ", size %08" LL_FMT_EA_T ", insn: %s\n", ea, size, dis.c_str());
}

void Labeless::onMakeData(ea_t ea, ::flags_t flags, ::tid_t, ::asize_t len)
{
	Q_UNUSED(len);
	if (m_IgnoreMakeData)
		return;
	if ((!inf.is_64bit() && !isDwrd(flags)) || (inf.is_64bit() && !isQwrd(flags)))
		return;

	//msg("on make_data: ea: %08" LL_FMT_EA_T ", flags: %08X, len: %08" LL_FMT_EA_T "\n", ea, flags, len);

	const uval_t val = ::inf.is_64bit() ? get_qword(ea) : get_long(ea);

	auto it = m_ExternRefsMap.find(val);
	if (it == m_ExternRefsMap.end())
		return;
	auto p = it->second.find('.');
	if (p == it->second.npos)
	{
		msg("%s: Wrong internal data for ea: %08" LL_FMT_EA_T "\n", __FUNCTION__, ea);
		return;
	}
	const std::string& procName = it->second.substr(p + 1);
	qstring name;
	if (get_ea_name(&name, ea) && name == procName.c_str())
		return;
	if (ASKBTN_YES != askyn_c(ASKBTN_NO, "Seems like address to API: \"%s\"\nMake it external now?", it->second.c_str()))
		return;

	if (!do_name_anyway(ea, procName.c_str()))
	{
		msg("%s: do_name_anyway() failed for ea: %08" LL_FMT_EA_T "\n", __FUNCTION__, ea);
		set_cmt(ea, procName.c_str(), false);
	}

	if (!op_enum(ea, 0, get_enum(kAPIEnumName.c_str()), 0))
		msg("%s: op_enum() failed for ea: %08" LL_FMT_EA_T "\n", __FUNCTION__, ea);
}

void Labeless::onAddCref(ea_t from, ea_t to, cref_t type)
{
	Q_UNUSED(from);
	Q_UNUSED(to);
	Q_UNUSED(type);
	//msg("on add_cref (from: %08" LL_FMT_EA_T ", to: %08" LL_FMT_EA_T ", type: %08X)\n", from, to, type);
}

void Labeless::onAddDref(ea_t from, ea_t to, dref_t type)
{
	Q_UNUSED(from);
	Q_UNUSED(to);
	Q_UNUSED(type);
	//msg("on add_Dref (from: %08" LL_FMT_EA_T ", to: %08" LL_FMT_EA_T ", type: %08X)\n", from, to, type);
}

bool Labeless::make_dword_ptr(ea_t ea, asize_t size)
{
	ScopedEnabler enabler(m_IgnoreMakeData);
	if (::inf.is_64bit())
		return doQwrd(ea, size);

	return doDwrd(ea, size);
}

int idaapi Labeless::ui_callback(void*, int notification_code, va_list va)
{
	if (notification_code == ui_tform_visible)
	{
		TForm* const form = va_arg(va, TForm *);

		if (m_EditorTForm && form == m_EditorTForm)
		{
			Labeless& ll = instance();
			QWidget* const w = reinterpret_cast<QWidget *>(form);
			QHBoxLayout* const mainLayout = new QHBoxLayout(w);
			mainLayout->setMargin(0);
			if (!ll.m_PyOllyView)
			{
				ll.m_PyOllyView = new PyOllyView(ll.isShowAllResponsesInLog());
				CHECKED_CONNECT(connect(ll.m_PyOllyView.data(), SIGNAL(runScriptRequested()),
					&ll, SLOT(onRunScriptRequested())));
				CHECKED_CONNECT(connect(ll.m_PyOllyView.data(), SIGNAL(settingsRequested()),
					&ll, SLOT(onSettingsRequested())));
				CHECKED_CONNECT(connect(ll.m_PyOllyView.data(), SIGNAL(showAllResponsesInLogToggled(bool)),
					&ll, SLOT(setShowAllResponsesInLog(bool))));
				CHECKED_CONNECT(connect(ll.m_PyOllyView.data(), SIGNAL(anchorClicked(const QString&)),
					&ll, SLOT(onLogAnchorClicked(const QString&))));
				CHECKED_CONNECT(connect(ll.m_PyOllyView.data(), SIGNAL(clearLogsRequested()),
					&ll, SLOT(onClearLogsRequested())));
			}
			mainLayout->addWidget(ll.m_PyOllyView.data());
			w->setLayout(mainLayout);
			w->setWindowIcon(QIcon(":/python.png"));
		}
		return 0;
	}
	if (notification_code == ui_plugin_loaded)
	{
		static const std::string kIDAPython = "IDAPython";
		::plugin_info_t* info = va_arg(va, ::plugin_info_t *);
		std::string name = info->name;
		if (name == kIDAPython)
			Labeless::instance().initIDAPython();
		
		return 0;
	}
	if (notification_code == ui_tform_invisible)
	{
		TForm *form = va_arg(va, TForm *);
		if (form == m_EditorTForm)
		{
			instance().onPyOllyFormClose();
		}
		return 0;
	}
	if (notification_code == ui_mbox)
	{
		//::mbox_kind_t kind = va_arg(va, ::mbox_kind_t);
		if (instance().m_SuppressMessageBoxesFromIDA)
			return -1;
		return 0;
	}
	return 0;
}

int idaapi Labeless::idp_callback(void* /*user_data*/, int notification_code, va_list va)
{
	Labeless& ll = instance();
	switch (notification_code)
	{
	case ::processor_t::newfile:
	case ::processor_t::oldfile:
		do {
			const bool compat = ph.id == PLFM_386 && inf.filetype == f_PE;
			if (compat)
			{
				ll.setEnabled();
				ll.enableMenuActions(true);
			}
		} while (0);
		return 0;
	case ::processor_t::closebase:
		ll.terminate();
		ll.enableMenuActions(false);
		return 0;
	default:
		break;
	}
	if (!ll.m_Enabled)
		return 0;

	switch (notification_code)
	{
	case ::processor_t::init:
		break;
	case ::processor_t::term:
		break;
	case ::processor_t::rename:
		do {
			ea_t ea = va_arg(va, ea_t);
			const char* newName = va_arg(va, const char*);
			// int flags = va_arg(va, int);
			ll.onRename(ea, newName);
		} while (0);
		break;
	/*case ::processor_t::auto_queue_empty:
		do {
			::atype_t t = va_arg(va, ::atype_t);
			if (t == AU_FINAL)
				ll.onAutoanalysisFinished();
			//if (t == AU_CODE || t == AU_WEAK)
			//	msg("auto_queue_empty: %u\n", t);
		} while (0);
		return 1;*/
	case ::processor_t::gen_regvar_def:
		do {
			::regvar_t *v = va_arg(va, ::regvar_t *);
			msg("on gen_regvar_def: ea: %08" LL_FMT_EA_T ", to: %08" LL_FMT_EA_T ", canon: %s, user: %s, cmt: %s\n", v->startEA, v->endEA, v->canon, v->user, v->cmt);
		} while (0);
		break;
	case ::processor_t::make_code:
		do {
			ea_t ea = va_arg(va, ea_t);
			::asize_t size = va_arg(va, ::asize_t);
			ll.onMakeCode(ea, size);
		} while (0);
		break;
	case ::processor_t::make_data:
		do {
			::ea_t ea = va_arg(va, ::ea_t);
			::flags_t flags = va_arg(va, ::flags_t);
			::tid_t tid = va_arg(va, ::tid_t);
			::asize_t len = va_arg(va, ::asize_t);

			ll.onMakeData(ea, flags, tid, len);
		} while (0);
		break;
	case ::processor_t::add_cref:
		break;
		do {
			::ea_t from = va_arg(va, ::ea_t);
			::ea_t to = va_arg(va, ::ea_t);
			::cref_t type = va_arg(va, ::cref_t);
			ll.onAddCref(from, to, type);
		} while (0);
#ifdef __NT__
	case ::processor_t::add_dref:
		do {
			::ea_t from = va_arg(va, ::ea_t);
			::ea_t to = va_arg(va, ::ea_t);
			dref_t type = va_arg(va, ::dref_t);
			ll.onAddDref(from, to, type);
		} while (0);
		break;
#endif // __NT__
	// TODO: disable on-rename
	//case ::processor_t::moving_segm:
	//	msg("on moving_segm\n");
	//	break;
	//case ::processor_t::move_segm:
	//	msg("on move_segm\n");
	//	break;
	}

	return 0;
}

int idaapi Labeless::idb_callback(void* /*user_data*/, int notification_code, va_list va)
{
	Q_UNUSED(notification_code);
	Q_UNUSED(va);
	/*switch (notification_code)
	{
	case ::idb_event::cmt_changed:
		do {
			ea_t ea = va_arg(va, ::ea_t);
			bool rpt = va_arg(va, bool);
			msg("cmt_changed at %" FMT_EA " is_rpt: %s\n", ea, rpt ? "yes":"no"); // TODO
		} while (0);
		break;
	}*/
	return 0;
}

void Labeless::onWipeAndImportRequested()
{
	importCode(true);
}

void Labeless::onKeepAndImportRequested()
{
	importCode(false);
}

bool Labeless::initIDAPython()
{
	return util::idapython::init();
}

void Labeless::onLogMessage(const QString& message, const QString& prefix)
{
	msg("%s: %s\n", prefix.toStdString().c_str(), message.toStdString().c_str());
}

void Labeless::addLogItem(LogItem& logItem)
{
	if (!m_EditorTForm || !m_PyOllyView)
		return;

	static uint64_t number;
	logItem.number = ++number;

	m_LogItems.insert(logItem.number, logItem);

	static const QString stdOutFmt = tr("<a href=\"%1\" title=\"Click to load into editors\">request #%2</a> %3<br>");

	m_PyOllyView->prependStdoutLog("-------------------------------------------------------------------<br>", true);
	m_PyOllyView->prependStdoutLog(stdOutFmt
		.arg(logItem.number)
		.arg(logItem.number)
		.arg(logItem.textStdErr.isEmpty() ? QString::null : tr("(has std_err |&gt;)")), true);
	if (!logItem.textStdOut.isEmpty())
	{
		m_PyOllyView->prependStdoutLog("<br>", true);
		m_PyOllyView->prependStdoutLog(logItem.textStdOut);
	}

	m_PyOllyView->prependStderrLog("-------------------------------------------------------------------<br>", true);
	m_PyOllyView->prependStderrLog(stdOutFmt
		.arg(logItem.number)
		.arg(logItem.number)
		.arg(logItem.textStdOut.isEmpty() ? QString::null : tr("(has std_out &lt;|)")), true);
	if (!logItem.textStdErr.isEmpty())
	{
		m_PyOllyView->prependStderrLog("<br>", true);
		m_PyOllyView->prependStderrLog(logItem.textStdErr);
	}
}

void Labeless::onLogAnchorClicked(const QString& value)
{
	if (!m_PyOllyView)
		return;

	bool ok = false;
	const uint64_t number = value.toULongLong(&ok);
	if (!ok || !m_LogItems.contains(number))
		return;

	const LogItem& li = m_LogItems[number];

	if (li.idaScript.isEmpty() && li.ollyScript.isEmpty())
		return;

	if (ASKBTN_YES != askyn_c(ASKBTN_NO, "Do you want to load IDA & Olly scripts related to that request into editors?\n"
			"All changes made will be lost!"))
		return;

	m_PyOllyView->setIDAScript(li.idaScript);
	m_PyOllyView->setOllyScript(li.ollyScript);
}

void Labeless::onClearLogsRequested()
{
	m_LogItems.clear();
}

void Labeless::onRpcRequestFailed(QString message)
{
	if (!message.isEmpty())
		info("RPC request failed with error:\n%s", message.toStdString().c_str());
}

