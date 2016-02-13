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
#include <ida.hpp>
#include <idp.hpp>

#include <auto.hpp>
#include <allins.hpp>
#include <bytes.hpp>
#include <dbg.hpp>
#include <demangle.hpp>
#include <entry.hpp>
#include <enum.hpp>
#include <expr.hpp>
#include <fixup.hpp>
#include <frame.hpp>
#include <kernwin.hpp>
#include <loader.hpp>
#include <loader.hpp>
#include <nalt.hpp>
#include <name.hpp>
#include <offset.hpp>
#include <segment.hpp>
#include <srarea.hpp>
#include <typeinf.hpp>
#include <../ldr/idaldr.h>

// std
#include <algorithm>
#include <deque>
#include <set>
#include <sstream>
#include <mstcpip.h>
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
#include <QTextCodec>
#include <QVariant>

#include <google/protobuf/stubs/common.h>

#include "hlp.h"
#include "rpcthreadworker.h"
#include "../common/cpp/rpc.pb.h"
#include "../common/version.h"
#include "choosememorydialog.h"
#include "globalsettingsmanager.h"
#include "idadump.h"
#include "pyollyview.h"
#include "pythonpalettemanager.h"
#include "settingsdialog.h"

namespace {


struct ScopedEnabler
{
	QAtomicInt& ref;
	ScopedEnabler(QAtomicInt& ref_)
		: ref(ref_)
	{
		ref = 1;
	}
	~ScopedEnabler()
	{
		ref = 0;
	}
};

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


static const uint32_t kDefaultExternSegLen = 0xF000;
static const Settings kDefaultSettings(
	"127.0.0.1",
	3852,
	0,
	false,
	true,
	true,
	true,
	true,
	true,
	kDefaultExternSegLen,
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

static const std::string kNetNodeLabeless = "$ labeless";
static const std::string kNetNodeExternSegData = "$ externsegdata";
static const std::string kNetNodeExternSegImps = "$ externsegimps";
static const std::string kAPIEnumName = "OLD_API_EXTERN_CONSTS";
static const QString kLabelessMenuObjectName = "labeless_menu";
static const QString kLabelessMenuLoadStubItemName = "act-load-stub";



} // anonymous

TForm* Labeless::m_EditorTForm;

Labeless::Labeless()
	: m_Initialized(false)
	, m_ConfigLock(QMutex::Recursive)
	, m_ThreadLock(QMutex::Recursive)
	, m_SynchronizeAllNow(false)
	, m_LabelSyncOnRenameIfZero(0)
	, m_ShowAllResponsesInLog(true)
{
	msg("%s\n", __FUNCTION__);
	WSADATA wd = {};
	WSAStartup(MAKEWORD(2, 2), &wd);

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
	netnode n;
	const bool nodeExists = !n.create(kNetNodeLabeless.c_str());
	GlobalSettingsManger& gsm = GlobalSettingsManger::instance();

	do {
		if (nodeExists)
		{
			char buff[MAXSTR] = {};
			if (n.supstr(0, buff, MAXSTR) >= 2)
				m_Settings.host = buff;
			m_Settings.port = n.altval(LNAT_Port);
			m_Settings.remoteModBase = n.altval(LNAT_RmoteModBase);
			m_Settings.demangle = n.altval(LNAT_Demangle) != 0;
			m_Settings.localLabels = n.altval(LNAT_LocalLabels) != 0;
			m_Settings.commentsSync = static_cast<Settings::CommentsSync>(n.altval(LNAT_CommentsSync));
		}
		else
		{
			m_Settings.host = gsm.value(GSK_PrevSelectedOllyHost, QString::fromStdString(m_Settings.host)).toString().toStdString();
			m_Settings.remoteModBase = get_imagebase();
		}

		m_Settings.analysePEHeader = gsm.value(GSK_AnalyzePEHeader, m_Settings.analysePEHeader).toBool();
		m_Settings.defaultExternSegSize = gsm.value(GSK_DefaultExternSegSize, m_Settings.defaultExternSegSize).toUInt();
		m_Settings.postProcessFixCallJumps = gsm.value(GSK_PostProcessFixCallJumps, m_Settings.postProcessFixCallJumps).toBool();
		m_Settings.overwriteWarning = static_cast<Settings::OverwriteWarning>(gsm.value(GSK_OverwriteWarning, m_Settings.overwriteWarning).toInt());
	} while (0);

	if (!nodeExists)
		storeSettings();

	return m_Settings;
}

void Labeless::storeSettings()
{
	netnode n;
	n.create(kNetNodeLabeless.c_str());

	n.supset(0, m_Settings.host.c_str());
	n.altset(LNAT_Port, m_Settings.port);
	n.altset(LNAT_RmoteModBase, m_Settings.remoteModBase);
	n.altset(LNAT_Demangle, m_Settings.demangle ? 1 : 0);
	n.altset(LNAT_LocalLabels, m_Settings.localLabels ? 1 : 0);
	n.altset(LNAT_CommentsSync, m_Settings.commentsSync);

	// global settings
	do {
		auto& gsm = GlobalSettingsManger::instance();
		gsm.setValue(GSK_PrevSelectedOllyHost, QString::fromStdString(m_Settings.host));
		gsm.setValue(GSK_AnalyzePEHeader, m_Settings.analysePEHeader);
		gsm.setValue(GSK_DefaultExternSegSize, m_Settings.defaultExternSegSize);
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

void Labeless::setTargetHostAddr(const std::string& ip, WORD port)
{
	QMutexLocker lock(&m_ConfigLock);

	m_Settings.host = ip;
	m_Settings.port = port;
}

void Labeless::onSyncronizeAllRequested()
{
	if (!m_Settings.remoteModBase && QMessageBox::No == QMessageBox::question(findIDAMainWindow(), tr("?"),
			tr("The \"Remote module base\" is zero.\nDo you want to continue?"),
			QMessageBox::Yes, QMessageBox::No))
		return;

	m_SynchronizeAllNow = true;
	msg("Labeless: do sync all now...\n");

	LabelsSync::DataList labelPoints;
	CommentsSync::DataList commentPoints;

	qstring name;
	int flags = 0;
	if (m_Settings.localLabels)
		flags |= GN_LOCAL;
	if (m_Settings.demangle)
		flags |= GN_VISIBLE | GN_DEMANGLED | GN_SHORT;

	char segBuff[MAXSTR];

	segment_t* s = static_cast<segment_t*>(segs.first_area_ptr());
	for (; s; s = static_cast<segment_t*>(segs.next_area_ptr(s->startEA)))
	{
		if (is_spec_segm(s->type))
			continue;
		const bool isExternSeg = (s->type & SEG_XTRN) == SEG_XTRN && get_segm_name(s, segBuff, MAXSTR) > 0 && std::string(segBuff) == NAME_EXTERN;
		if (isExternSeg)
			continue;
		ea_t ea = s->startEA;

		while (true)
		{
			if (get_true_name(&name, ea, flags) > 0 && is_uname(name.c_str()))
			{
				std::string s = name.c_str();
				if (s.length() >= OLLY_TEXTLEN)
					s.erase(s.begin() + OLLY_TEXTLEN - 1, s.end());
				if (isUtf8StringValid(s.c_str(), s.length()))
					labelPoints.push_back(LabelsSync::Data(ea, s));
				//msg("%08X: %s\n", ea, name.c_str());
			}
			
			ea = hlp::getNextCodeOrDataEA(ea, m_Settings.nonCodeNames);
			if (BADADDR == ea)
				break;
			//auto sel_ea = sel2ea(s->sel);
		}
	}

	if (!labelPoints.empty())
		addLabelsSyncData(labelPoints);

	if (m_Settings.commentsSync != Settings::CS_Disabled)
	{
		char buff[OLLY_TEXTLEN];
		const bool nonRptCmt = m_Settings.commentsSync == Settings::CS_NonRepeatable || m_Settings.commentsSync == Settings::CS_All;
		const bool rptCmt = m_Settings.commentsSync == Settings::CS_Repeatable || m_Settings.commentsSync == Settings::CS_All;
		ssize_t len;

		s = static_cast<segment_t*>(segs.first_area_ptr());
		for (; s; s = static_cast<segment_t*>(segs.next_area_ptr(s->startEA)))
		{
			for (ea_t ea = s->startEA, endEa = s->endEA; ea < endEa; ++ea)
			{
				if (!has_cmt(getFlags(ea)))
					continue;

				if (nonRptCmt &&
					(len = get_cmt(ea, false, buff, OLLY_TEXTLEN)) > 0 &&
					isUtf8StringValid(buff, len))
				{
					commentPoints.append(CommentsSync::Data(ea, std::string(buff, static_cast<size_t>(len))));
					continue;
				}
				if (rptCmt &&
					(len = get_cmt(ea, true, buff, OLLY_TEXTLEN)) > 0 &&
					isUtf8StringValid(buff, len))
					commentPoints.append(CommentsSync::Data(ea, std::string(buff, static_cast<size_t>(len))));
			}
		}
		if (!commentPoints.isEmpty())
			addCommentsSyncData(commentPoints);
	}
	msg("Labels: %d, comments: %d\n", labelPoints.count(), commentPoints.count());
	m_SynchronizeAllNow = false;
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
		m_MenuActions.at(i)->setEnabled(m_MenuActions.at(i)->objectName() == kLabelessMenuLoadStubItemName ? !enabled: enabled);
}

std::string Labeless::ollyHost() const
{
	QMutexLocker lock(&m_ConfigLock);
	return m_Settings.host;
}

WORD Labeless::ollyPort() const
{
	QMutexLocker lock(&m_ConfigLock);
	return m_Settings.port;
}

void Labeless::onRename(uint32_t ea, const std::string& newName)
{
	//if (m_LabelSyncOnRenameIfZero)
	//	return;
	if (!m_Settings.enabled)
		return;
	if (!m_DumpList.isEmpty() && m_DumpList.last().state != IDADump::ST_Done)
		return;
	msg("%s: rename addr %08X to %s\n", __FUNCTION__, ea, newName.c_str());
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
	
	char mnemBuff[MAXSTR] = {};
	char disasm[MAXSTR] = {};

	const QRegExp reOpnd("\\s*(dword|near|far)\\s+ptr\\s+([\\w]+)\\+([a-f0-9]+)h?\\s*", Qt::CaseInsensitive);
	const QRegExp reShortOpnd("\\s*\\$\\+([a-f0-9]+)h?\\s*", Qt::CaseInsensitive);
	foreach(ReadMemoryRegions::t_memory m, dump.readMemRegions->data)
	{
		segment_t* const seg = getseg(m.base);

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
				msg("%s: found TODO for fix at: 0x%08X, opnd:%s\n", __FUNCTION__, ea, disasm);
				if (do_unknown(target, 0) && create_insn(target))
					msg("%s: ea: 0x%08X fixed\n", __FUNCTION__, ea);
			}
			else if (reShortOpnd.exactMatch(sDisasm))
			{
				msg("%s: found TODO for fix 2 at: 0x%08X, opnd: %s\n", __FUNCTION__, ea, disasm);
			}
		}
	}

	dump.nextState(nullptr);
}

bool Labeless::firstInit()
{
	if (QMainWindow* mw = findIDAMainWindow())
	{
		if (QMenu* m = mw->menuBar()->addMenu("Labeless"))
		{
			m->setObjectName(kLabelessMenuObjectName);
			QAction* actLoadStub = m->addAction(tr("Load stub database..."), this, SLOT(onLoadStubDBRequested()));
			actLoadStub->setObjectName(kLabelessMenuLoadStubItemName);
			m_MenuActions << actLoadStub;
			m->addSeparator();
			m_MenuActions << m->addAction(QIcon(":/run.png"), tr("Remote Python execution"), this, SLOT(onShowRemotePythonExecutionViewRequested()));
			QMenu* dumpMenu = m->addMenu(QIcon(":/dump.png"), tr("IDADump"));
			m_MenuActions << dumpMenu->addAction(tr("Wipe all and import..."), this, SLOT(onWipeAndImportRequested()));
			m_MenuActions << dumpMenu->addAction(tr("Keep existing and import..."), this, SLOT(onKeepAndImportRequested()));
			m_MenuActions << m->addAction(QIcon(":/sync.png"), tr("Sync labels now"), this, SLOT(onSyncronizeAllRequested()));
			m->addSeparator();
			m_MenuActions << m->addAction(QIcon(":/settings.png"), tr("Settings..."), this, SLOT(onSettingsRequested()));
		}
	}
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

	QMutexLocker lock(&m_ThreadLock);
	m_Thread = QPointer<QThread>(new QThread(this));
	RpcThreadWorker* worker = new RpcThreadWorker();
	CHECKED_CONNECT(connect(m_Thread.data(), SIGNAL(started()), worker, SLOT(main())));
	CHECKED_CONNECT(connect(worker, SIGNAL(destroyed()), m_Thread.data(), SLOT(quit()), Qt::QueuedConnection));
	CHECKED_CONNECT(connect(m_Thread.data(), SIGNAL(finished()), worker, SLOT(deleteLater())));
	worker->moveToThread(m_Thread.data());
	m_Thread->start();
	m_Initialized = true;

	return true;
}

void Labeless::terminate()
{
	if (!m_Initialized)
		return;
	m_Initialized = false;

	if (m_PyOllyView && m_EditorTForm)
		m_PyOllyView->close();

	if (m_Enabled)
		m_Enabled = 0;

	QMutexLocker lock(&m_ThreadLock);
	if (m_Thread)
	{
		m_QueueCond.wakeAll();
		QEventLoop loop;
		loop.processEvents(QEventLoop::AllEvents, 2000);
		m_Thread->terminate();
		m_Thread->wait();
		m_Thread = nullptr;
	}
	lock.unlock();
	m_Queue.clear();

	m_ExternSegData = ExternSegData();
}

void Labeless::shutdown()
{
	static bool shutdownCalled = false;
	if (shutdownCalled)
		return;
	shutdownCalled = true;
	terminate();
	WSACleanup();
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

SOCKET Labeless::connectToHost(const std::string& host, uint16_t port, QString& errorMsg, bool keepAlive /*= true*/)
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
				.arg(hlp::net::wsaErrorToString().c_str());
			return INVALID_SOCKET;
		}
		addr.sin_addr.s_addr = *(ulong*)he->h_addr_list[0];
	}
	if (addr.sin_addr.s_addr == INADDR_NONE || addr.sin_addr.s_addr == INADDR_ANY)
		return INVALID_SOCKET;

	SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		errorMsg = QString("%1: socket() failed. Error %2\n").arg(__FUNCTION__).arg(hlp::net::wsaErrorToString().c_str());
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
				errorMsg = QString("%1: setsockopt(SO_KEEPALIVE) failed. LE: %2\n").arg(__FUNCTION__).arg(hlp::net::wsaErrorToString().c_str());
				break;
			}
		}
		const DWORD recvTimeout = 30 * 60 * 1000;
		if (SOCKET_ERROR == ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&recvTimeout), sizeof(recvTimeout)))
		{
			errorMsg = QString("%1: setsockopt(SO_RCVTIMEO) failed. LE: %2\n").arg(__FUNCTION__).arg(hlp::net::wsaErrorToString().c_str());
			break;
		}

		static const tcp_keepalive keepAliveCfg = { 1, 30 * 60 * 1000, 2000 };
		if (keepAlive)
		{
			DWORD dwDummy = 0;
			if (SOCKET_ERROR == ::WSAIoctl(s, SIO_KEEPALIVE_VALS, LPVOID(&keepAliveCfg), sizeof(keepAliveCfg), nullptr, 0, &dwDummy, nullptr, nullptr))
			{
				errorMsg = QString("%1: WSAIoctl(SIO_KEEPALIVE_VALS) failed. LE: %2\n").arg(__FUNCTION__).arg(hlp::net::wsaErrorToString().c_str());
				break;;
			}
		}
		if (SOCKET_ERROR == ::connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
		{
			errorMsg = QString("%1: connect() to %2:%3 failed. Error: %4\n")
				.arg(__FUNCTION__)
				.arg(QString::fromStdString(host))
				.arg(port)
				.arg(hlp::net::wsaErrorToString().c_str());
			break;
		}
		failed = false;
	} while (0);
	if (failed)
	{
		::closesocket(s);
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
	QMainWindow* idaw = findIDAMainWindow();
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
	/*if (::is_debugger_on())
	{
		QMessageBox::information(findIDAMainWindow(), tr("!"), tr("Stop debugging first"));
		return;
	}
	if (::dbg && !dbg->version)
	{
		term_plugins(PLUGIN_DBG);
	}*/
	if (!::is_temp_database())
	{
		const auto rv = QMessageBox::question(findIDAMainWindow(), tr("?"), tr("Do you really want to load sample DB?"),
				QMessageBox::Yes, QMessageBox::No);
		if (rv != QMessageBox::Yes)
			return;
	}

	QString idbFileName = QFileDialog::getSaveFileName(findIDAMainWindow(), tr("Select where to save new DB"), "sample.idb",
			tr("IDA PRO IDB-file (*.idb)"));
	if (idbFileName.isEmpty())
		return;
	if (!idbFileName.endsWith(".idb", Qt::CaseInsensitive))
		idbFileName.append(".idb");
	const QString sampleFileName = idbFileName.left(idbFileName.length() - 4) + ".exe";

	const std::string sfn = sampleFileName.toStdString();

	QByteArray raw;
	do 
	{
		QFile fsrc(":/stub.exe.bin");
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
		QMessageBox::critical(findIDAMainWindow(), tr("!"), tr("Unable to open file \"%1\" for write").arg(sampleFileName));
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
		open_tform(m_EditorTForm, FORM_TAB | FORM_MENU | FORM_RESTORE | FORM_QWIDGET);
	else
		openPythonEditorForm(FORM_TAB | FORM_MENU | FORM_RESTORE);
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
		if (!runIDAPythonScript(idaScript, externObj, errorMsg))
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
	// TODO
}

bool Labeless::importCode(bool wipe)
{
	auto req = std::make_shared<GetMemoryMapReq>();

	RpcDataPtr p = addRpcData(req, RpcReadyToSendHandler(), this, SLOT(onGetMemoryMapFinished()));
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
	ChooseMemoryDialog cmd(vals, tr("Select memory to dump"), findIDAMainWindow());
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
		msg("addr: %08X, size: %08X, protect: %08X\n", v.base, v.size, v.protect);
		rmr->data.push_back(ReadMemoryRegions::t_memory(v.base, v.size, v.protect, ""));
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

		if (!wipe)
		{
			bool snapshotTaken = false;
			// check is overlaps
			area_t area(base, endEa);
			for (auto it = existingSegs.cbegin(), end = existingSegs.cend(); it != end; ++it)
			{
				segment_t* seg = *it;
				if (seg->overlaps(area))
				{
					static const int kOverwrite						= 1 << 0;
					static const int kTakeSnapAndOverwrite			= 1 << 1;
					static const int kAlwaysTakeSnapAndOvrDontAsk	= 1 << 2;
					static const int kAlwaysOvrDontAsk				= 1 << 3;
					int chTakeSnapshot = 1;

					if (Settings::OW_WarnAndCancel == m_Settings.overwriteWarning)
					{
						QMessageBox::warning(findIDAMainWindow(), tr("!"),
							tr("One of selected regions[%1, %2) overlaps with existing segment[%3, %4)."
								"Exiting... The default behavior can be changed in Settings view.")
								.arg(area.startEA, 8, 16, QChar('0')).arg(area.endEA, 8, 16, QChar('0'))
								.arg(seg->startEA, 8, 16, QChar('0')).arg(seg->endEA, 8, 16, QChar('0')));
						return;
					}
					if (Settings::OW_AlwaysAsk == m_Settings.overwriteWarning)
					{
						const QString fmt = QString("?\n\n\n"
							"One of selected regions[%1, %2) overlaps with existing segment[%3, %4).\n"
							"Do you want to overwrite/extend existing segment?"
							"\n"
							"<Overwrite :R>\n"
							"<Take IDB snapshot & overwrite :R>"
							"<Always take snapshot, don't ask :R>"
							"<Always overwrite, don't ask :R>>")
								.arg(area.startEA, 8, 16, QChar('0')).arg(area.endEA, 8, 16, QChar('0'))
								.arg(seg->startEA, 8, 16, QChar('0')).arg(seg->endEA, 8, 16, QChar('0'));

						if (!AskUsingForm_c(fmt.toStdString().c_str(), &chTakeSnapshot))
							return;

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
						char ss_date[MAXSTR];
						qstrftime64(ss_date, sizeof(ss_date), "%Y-%m-%d %H:%M:%S", qtime64());

						::qstrncpy(snap.desc, kSnapshotDescriptionFmt.arg(ss_date).toStdString().c_str(), sizeof(snap.desc));
						snapshotTaken = take_database_snapshot(&snap, &errmsg);
						if (!snapshotTaken)
							msg("%s: Unable to take database snapshot, error: %s\n", __FUNCTION__, errmsg.c_str());
						else
							msg("%s: Database snapshot taken (%s)\n", __FUNCTION__, snap.filename);
					}
				}
			}
		}

		m_DumpList.append(dump);

		RpcDataPtr p = m_DumpList.back().nextState(nullptr).state == IDADump::ST_CheckingPEHeader
			? addRpcData(dump.checkPEHeaders, RpcReadyToSendHandler(), this, SLOT(onCheckPEHeadersFinished()))
			: addRpcData(dump.readMemRegions, RpcReadyToSendHandler(), this, SLOT(onReadMemoryRegionsFinished()));
		p->setProperty("wipe", dump.wipe);
	}
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
		msg("\n");
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
			ea_t newBase = rmr->data.front().base;
			set_imagebase(newBase);
			msg("ImageBase automatically changed to %08X because 'wipe' is requested\n", newBase);
			m_Settings.remoteModBase = newBase;
			storeSettings();
		}
	}

	m_LabelSyncOnRenameIfZero = rmr->data.size();

	if (!rmr->data.empty())
	{
		static const ea_t kReservedFreeSpace = 0x1000;
		const ea_t impSegFrom = MAXADDR - m_Settings.defaultExternSegSize - kReservedFreeSpace;
		const ea_t impSegTo = MAXADDR - kReservedFreeSpace;

		if (!createImportSegment(impSegFrom, impSegTo))
		{
			msg("%s: Unable to create import segment from: 0x%08X to 0x%08X\n",
				__FUNCTION__, impSegFrom, impSegTo);
			// TODO: may be fail
			return;
		}
	}

	const ea_t region_base = rmr->data.front().base;
	const uint32_t region_size = rmr->data.back().base + rmr->data.back().size - region_base;

	icInfo.nextState(nullptr);

	for (int i = 0; i < rmr->data.size(); ++i)
	{
		const ReadMemoryRegions::t_memory& m = rmr->data.at(i);
		if (m.raw.size() != m.size)
		{
			msg("%s: Raw data size mismatch (expected %08X, received %08X)\n", __FUNCTION__, m.size, m.raw.size());
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

	if (sectionIt != icInfo.sections.constEnd())
	{
		msg("Section found for region{ ea: %08X, size: %08X } is section { name: %s, va: %08X, size: %08X, ch: %08X }\n",
			area.startEA, area.size(), sectionIt->name.c_str(), sectionIt->va, sectionIt->va_size, sectionIt->characteristics);

		const uint32_t ch = sectionIt->characteristics;
		type = (ch & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE))
			? SEG_CODE
			: SEG_DATA;
		perm =
			((ch & IMAGE_SCN_MEM_WRITE) ? SEGPERM_WRITE : 0) +
			((ch & IMAGE_SCN_MEM_READ) ? SEGPERM_READ : 0) +
			((ch & IMAGE_SCN_MEM_EXECUTE) ? SEGPERM_EXEC : 0);
		return;
	}
	const auto sProtect = hlp::memoryProtectToStr(m.protect);
	if (sProtect.find('E') != sProtect.npos)
		type = SEG_CODE;
	else
		type = SEG_DATA;

	if (sProtect.find('E') != sProtect.npos)
		perm += SEGPERM_EXEC;
	if (sProtect.find('W') != sProtect.npos)
		perm += SEGPERM_WRITE;
	if (sProtect.find('R') != sProtect.npos)
		perm += SEGPERM_READ;
}

bool Labeless::mergeMemoryRegion(IDADump& icInfo, const ReadMemoryRegions::t_memory& m, ea_t region_base, uint32_t region_size)
{
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

	result.bitness = 1;
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

	if (gdp->eip >= gdp->req.eaFrom && gdp->eip <= gdp->req.eaTo - sizeof(DWORD_PTR))
	{
		const auto& entryName = getNewNameOfEntry();
		add_entry(gdp->eip, gdp->eip, entryName.c_str(), true);
		msg("Entry created %s, waiting for finish anto-analysis\n", entryName.c_str());
		autoWait();
	}
	std::set<ea_t> exportEntries;
	for (int i = 0, e = icInfo.exports.size(); i < e; ++i)
	{
		auto ea = icInfo.exports.at(i).ea;
		if (ea >= gdp->req.base && ea <= gdp->req.eaTo)
		{
			exportEntries.insert(ea);
			add_entry(ea, ea, icInfo.exports.at(i).name.c_str(), true);
		}
	}
	char disasm[MAXSTR] = {};
	char disasmClean[MAXSTR] = {};
	for (int i = 0; i < gdp->ptrs.size(); ++i)
	{
		const AnalyzeExternalRefs::PointerData& pd = gdp->ptrs.at(i);
		const std::string joined = pd.module + "." + pd.procName;

		if (exportEntries.find(pd.ea) != exportEntries.end())
		{
			msg("entry %s already marked as external\n", joined.c_str());
			continue;
		}

		uint32_t indexInImportTable = 0;
		ea_t addr = 0;
		auto it = m_ExternSegData.imports.find(joined);
		if (it != m_ExternSegData.imports.end())
		{
			indexInImportTable = it->second.index;
			addr = m_ExternSegData.start + indexInImportTable * sizeof(DWORD_PTR);
		}
		else
		{
			indexInImportTable = m_ExternSegData.imports.size();
			addr = m_ExternSegData.start + indexInImportTable * sizeof(DWORD_PTR);
			ImportEntry ie;
			ie.index = indexInImportTable;
			ie.module = pd.module;
			ie.proc = pd.procName;
			if (!pd.procName.empty() && pd.procName[0] == '#')
				ie.ordinal = atol(pd.procName.substr(1).c_str());

			m_ExternRefsMap[pd.ea] = joined;
			m_ExternSegData.imports[joined] = ie;
			uval_t val = get_long(pd.ea);
			addAPIEnumValue(pd.module + "_" + pd.procName, val);

			do_unknown_range(addr, sizeof(DWORD_PTR), DOUNK_SIMPLE);
			if (!make_dword(addr, sizeof(DWORD_PTR)))
				msg("make_dword() failed for ea: %08X\n", addr);
			if (!set_name(addr, pd.procName.c_str(), SN_CHECK | SN_PUBLIC | SN_AUTO | SN_NOWARN) &&
				!do_name_anyway(addr, pd.procName.c_str()))
			{
				msg("set_name() and do_name_anyway() are failed for ea: %08X\n", addr);
				set_cmt(addr, pd.procName.c_str(), false);
			}

			do_unknown_range(pd.ea, sizeof(DWORD_PTR), DOUNK_DELNAMES);
			if (!make_dword(pd.ea, sizeof(DWORD_PTR)))
				msg("make_dword() failed for ea: %08X\n", pd.ea);

			if (!do_name_anyway(pd.ea, pd.procName.c_str()))
			{
				msg("do_name_anyway() failed for ea: %08X\n", pd.ea);
				set_cmt(pd.ea, pd.procName.c_str(), false);
			}

			if (!op_enum(pd.ea, 0, get_enum(kAPIEnumName.c_str()), 0))
				msg("op_enum() failed for ea: %08X\n", pd.ea);
		}

		qstring qnamee;
		get_ea_name(&qnamee, pd.ea);

		qstring addrStr;
		addrStr.sprnt("dword ptr [0%08Xh]", addr);
		ea_t drefEA = get_first_dref_to(pd.ea);
		if (qnamee.empty())
		{
			msg("error: qnamee empty: pd.ea: %08X, pd.proc: %s, pd.module: %s, drefEA: %08X\n",
				pd.ea, pd.procName.c_str(), pd.module.c_str(), drefEA);
			continue;
		}
		while (drefEA != BADADDR)
		{
			if (decode_insn(drefEA))
			{
				const insn_t c = ::cmd;
				if (generate_disasm_line(drefEA, disasm, MAXSTR, GENDSM_FORCE_CODE) &&
					tag_remove(disasm, disasmClean, MAXSTR))
				{
					qstring d (disasmClean);

					auto p = d.find(';');
					if (p != d.npos)
						d.resize(p);
					d.replace("dword ptr", "");
					d.replace(qnamee.c_str(), addrStr.c_str());
					::qstrncpy(disasmClean, d.c_str(), MAXSTR);
					char ass[MAXSTR] = {};
					int size = 0;

					do {
						ScopedEnabler enabler(m_SuppressMessageBoxesFromIDA);
						Q_UNUSED(enabler);
						size = ph.notify(ph.assemble, drefEA, ::cmd.cs, drefEA, true, disasmClean, ass);
					} while (0);

					if (size > 0 && static_cast<uint32_t>(size) <= ::cmd.size)
					{
						patch_many_bytes(drefEA, ass, size);
						if (static_cast<uint32_t>(size) < ::cmd.size)
						{
							for (int e = size; e < static_cast<int>(::cmd.size); ++e)
								patch_byte(drefEA + e, 0x90); // fill with nops
						}
					}
					else
					{
						msg("%s: failed to assembly command: %s at ea: %08X\n", __FUNCTION__, d.c_str(), drefEA);
					}
				}
			}
			drefEA = get_next_dref_to(pd.ea, drefEA);
		}
		reanalyze_callers(addr, false);
	}

	char buff[MAXSTR] = {};
	char bclean[MAXSTR] = {};
	for (int i = 0; i < gdp->rdl.size(); ++i)
	{
		const AnalyzeExternalRefs::RefData& rd = gdp->rdl.at(i);
		const ea_t ea = rd.instrEA;
		if (!isCode(get_flags_novalue(ea)))
		{
			const auto autoLen = create_insn(ea);
			if (autoLen != rd.len)
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

		if (!ua_mnem(ea, buff, MAXSTR) || !qstrupr(buff) || std::string(buff).substr(0, 1) != mnem.substr(0, 1))
			continue;

		const uint32_t val = rd.val;
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

void Labeless::openPythonEditorForm(int options /*= 0*/)
{
	HWND hwnd = NULL;
	m_EditorTForm = create_tform("PyOlly", &hwnd);
	open_tform(m_EditorTForm, FORM_TAB | FORM_MENU | FORM_RESTORE | FORM_QWIDGET);
}

void Labeless::onTestConnectRequested()
{
	SettingsDialog* sd = qobject_cast<SettingsDialog*>(sender());
	if (!sd)
		return;
	Settings tmpSettings;
	sd->getSettings(tmpSettings);
	QString error;
	if (testConnect(tmpSettings.host, tmpSettings.port, error))
		info("Successfully connected!");
	else
		info("Test failed, error: %s", error.toStdString().c_str());
}

bool Labeless::testConnect(const std::string& host, uint16_t port, QString& errorMsg)
{
	errorMsg.clear();
	SOCKET s = connectToHost(host, port, errorMsg, false);
	if (INVALID_SOCKET == s)
	{
		if (errorMsg.isEmpty())
			errorMsg = "connectToHost() failed\n";
		return false;
	}
	std::shared_ptr<void> guard (nullptr, [s](void*){ closesocket(s); }); // clean-up guard

	rpc::Execute command;
	command.set_script(
		"import sys\n"
		"from py_olly import labeless_ver\n"
		"print 'pong'\n"
		"print >> sys.stderr, 'v:%s' % labeless_ver()");
	if (!hlp::net::sockSendString(s, command.SerializeAsString()))
	{
		errorMsg = "sockSendString() failed";
		return false;
	}

	/*if (SOCKET_ERROR == shutdown(s, SD_SEND))
	{
		msg("shutdown() failed. Error: %s\n", hlp::net::wsaErrorToString());
		return false;
	}*/

	std::string rawResponse;
	if (!hlp::net::sockRecvAll(s, rawResponse) || rawResponse.empty())
	{
		errorMsg = QString("sockRecvAll() failed, error: %1").arg(hlp::net::wsaErrorToString().c_str());
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
		errorMsg = QString::fromStdString("version mismatch. Labeless IDA: " + kVer + ". But Labeless Olly: " + err);
	
	return rv;
}

uint32_t Labeless::loadImportTable()
{
	m_ExternSegData = ExternSegData();
	netnode n;
	if (!n.create(kNetNodeExternSegData.c_str()))
	{
		m_ExternSegData.start = n.altval(0);
		m_ExternSegData.len = n.altval(1);
		uint32_t importCount = n.altval(2);
		netnode n2;
		if (!n2.create(kNetNodeExternSegImps.c_str()))
		{
			char buff[MAXSTR] = {};
			for (uint32_t i = 0; i < importCount * 3; i += 3)
			{
				ImportEntry ie;
				std::string key;
				ie.index = i / 3;
				ie.ordinal = n2.altval(i);
				if (n2.supstr(i, buff, MAXSTR) > 0)
					ie.module = buff;
				if (n2.supstr(i + 1, buff, MAXSTR) > 0)
					ie.proc = buff;
				if (n2.supstr(i + 2, buff, MAXSTR) > 0)
					key = buff;
				if (!ie.module.empty() && (!ie.proc.empty() || ie.ordinal))
					m_ExternSegData.imports[key] = ie;
			}
			return m_ExternSegData.imports.size();
		}
	}
	return 0;
}

void Labeless::storeImportTable()
{
	netnode n;
	n.create(kNetNodeExternSegData.c_str());
	n.altset(0, m_ExternSegData.start);
	n.altset(1, m_ExternSegData.len);
	n.altset(2, m_ExternSegData.imports.size());
	netnode n2;
	n2.create(kNetNodeExternSegImps.c_str());
	
	for (auto it = m_ExternSegData.imports.cbegin(); it != m_ExternSegData.imports.cend(); ++it)
	{
		const uint32_t idx = it->second.index * 3;
		n2.supset(idx, it->second.module.c_str());
		n2.supset(idx + 1, it->second.proc.c_str());
		n2.supset(idx + 2, it->first.c_str());
		n2.altset(idx, it->second.ordinal);
	}
}

bool Labeless::createImportSegment(ea_t from, ea_t to)
{
	if (m_ExternSegData.start)
		return true;
	segment_t ns;
	segment_t *s = getseg(from);
	if (s != NULL)
		ns = *s;
	else
		ns.sel = setup_selector(0);

	ns.startEA = from;
	ns.endEA = to;
	ns.type = SEG_XTRN;
	ns.perm = SEGPERM_READ;
	ns.comb = scPub;
	ns.align = saRelPara;
	ns.color = DEFCOLOR;
	ns.bitness = 1; // 32
	ns.set_visible_segm(true);
	ns.set_loader_segm(true);
	bool ok = add_segm_ex(&ns, NAME_EXTERN, "XTRN", ADDSEG_NOSREG) != 0;
	if (ok)
	{
		m_ExternSegData.start = from;
		m_ExternSegData.len = to - from;
		msg("Import segment was created successfully\n");
	}

	return ok;
}

void Labeless::updateImportsNode()
{
	std::unordered_map<std::string, uval_t> module2Index;
	std::set<std::string> existingAPIs;

	char modname[MAXSTR + 4] = {};
	char funcName[MAXSTR];
	uval_t modulesCount = 0;
	for (uval_t idx = import_node.alt1st(); idx != BADNODE; idx = import_node.altnxt(idx))
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
			if (modNode.supstr(ea, funcName, sizeof(funcName)) > 0)
				existingAPIs.insert(funcName);
		}

		for (ea_t ea = modNode.sup1st(); ea != BADADDR; ea = modNode.supnxt(ea))
		{
			if (modNode.supstr(ea, funcName, sizeof(funcName)) > 0)
				existingAPIs.insert(funcName);
		}
	}

	auto it = m_ExternSegData.imports.cbegin();
	for (; it != m_ExternSegData.imports.cend(); ++it)
	{
		if (existingAPIs.count(it->second.proc))
			continue;
		netnode nModule;
		const bool moduleExists = module2Index.find(it->second.module) != module2Index.end();
		if (moduleExists)
		{
			nModule = import_node.altval(module2Index[it->second.module]);
		}
		else
		{
			qstring s;
			s.sprnt("$lib %s", it->second.module.c_str());
			nModule.create(s.c_str());
		}
		nModule.supset(m_ExternSegData.start + it->second.index * sizeof(DWORD_PTR), it->second.proc.c_str());
		if (it->second.ordinal)
		{
			nModule.altset(it->second.ordinal, m_ExternSegData.start + it->second.index * sizeof(DWORD_PTR));
			set_cmt(m_ExternSegData.start + it->second.index * sizeof(DWORD_PTR), (it->second.module + "." + it->second.proc).c_str(), true);
		}
		if (!moduleExists)
		{
			nodeidx_t lastFreeImpNode = import_node.altval(-1);
			import_node.altset(-1, lastFreeImpNode + 1);
			import_node.altset(lastFreeImpNode, nModule);
			import_node.supset(lastFreeImpNode, it->second.module.c_str());
			import_node.supstr(lastFreeImpNode, modname, sizeof(modname));
			module2Index[it->second.module] = lastFreeImpNode;
			modulesCount++;

			import_module(it->second.module.c_str(), nullptr, nModule, nullptr, "win");
			import_node.supstr(lastFreeImpNode, modname, sizeof(modname));
		}
		existingAPIs.insert(it->second.proc);
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
	//msg("on make_code: ea: %08X, size %08X, insn: %s\n", ea, size, dis.c_str());
}

void Labeless::onMakeData(ea_t ea, ::flags_t flags, ::tid_t, ::asize_t len)
{
	if (m_IgnoreMakeData)
		return;
	if (!isDwrd(flags) || len != sizeof(DWORD_PTR))
		return;

	//msg("on make_data: ea: %08X, flags: %08X, len: %08X\n", ea, flags, len);
	const auto val = get_long(ea);
	auto it = m_ExternRefsMap.find(val);
	if (it == m_ExternRefsMap.end())
		return;
	auto p = it->second.find('.');
	if (p == it->second.npos)
	{
		msg("Wrong internal data for ea: %08X\n", ea);
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
		msg("do_name_anyway() failed for ea: %08X\n", ea);
		set_cmt(ea, procName.c_str(), false);
	}

	if (!op_enum(ea, 0, get_enum(kAPIEnumName.c_str()), 0))
		msg("op_enum() failed for ea: %08X\n", ea);
}

void Labeless::onAddCref(ea_t from, ea_t to, cref_t type)
{
	//msg("on add_cref (from: %08X, to: %08X, type: %08X)\n", from, to, type);
}

void Labeless::onAddDref(ea_t from, ea_t to, dref_t type)
{
	//msg("on add_Dref (from: %08X, to: %08X, type: %08X)\n", from, to, type);
}

bool Labeless::make_dword(ea_t ea, asize_t size)
{
	ScopedEnabler enabler(m_IgnoreMakeData);
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
			int flags = va_arg(va, int);
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
			msg("on gen_regvar_def: ea: %08X, to: %08X, canon: %s, user: %s, cmt: %s\n", v->startEA, v->endEA, v->canon, v->user, v->cmt);
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
	case ::processor_t::add_dref:
		do {
			::ea_t from = va_arg(va, ::ea_t);
			::ea_t to = va_arg(va, ::ea_t);
			dref_t type = va_arg(va, ::dref_t);
			ll.onAddDref(from, to, type);
		} while (0);
		break;
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
	const extlang_t* elng = find_extlang_by_name("python");
	if (!elng)
	{
		msg("%s: python extlang not found\n", __FUNCTION__);
		return false;
	}
	char errbuff[1024] = {};
	static const std::string pyInitMsg = "import json\n"
		"idaapi.msg('Labeless: Python initialized... OK\\n')\n";
	if (!run_statements(pyInitMsg.c_str(), errbuff, _countof(errbuff), elng))
	{
		msg("%s: run_statements() failed\n", __FUNCTION__);
		if (::qstrlen(errbuff))
			msg("%s: error: %s", __FUNCTION__, errbuff);
		return false;
	}

	return true;
}

bool Labeless::runIDAPythonScript(const std::string& script, std::string& externObj, std::string& error)
{
	const extlang_t* elng = find_extlang_by_name("python");
	if (!elng)
	{
		msg("%s: Python extlang not found\n", __FUNCTION__);
		return false;
	}
	char errbuff[1024] = {};
	externObj.clear();
	error.clear();

	if (!run_statements(script.c_str(), errbuff, _countof(errbuff), elng))
	{
		if (::qstrlen(errbuff))
			error = errbuff;
		msg("%s: unable to execute Python script, error: %s", __FUNCTION__, errbuff);
		return false;
	}
	errbuff[0] = '\0';
	idc_value_t rv;
	if (elng->calcexpr(BADADDR, "json.dumps(__extern__)", &rv, errbuff, sizeof(errbuff)))
	{
		externObj = rv.c_str();
		VarFree(&rv);
	}
	else if (::qstrlen(errbuff) && !QString::fromLatin1(errbuff).contains("NameError"))
	{
		error = errbuff;
		return false;
	}
	return true;
}

QMainWindow* Labeless::findIDAMainWindow() const
{
	static QPointer<QMainWindow> mainWindow;
	if (mainWindow)
		return mainWindow;
	if (WId hwnd = reinterpret_cast<WId>(callui(ui_get_hwnd).vptr))
	{
		if (mainWindow = qobject_cast<QMainWindow*>(QWidget::find(hwnd)))
			return mainWindow;
	}
#if 0
	static const QString kIDAMainWindowClassName = "IDAMainWindow";

	QWidgetList wl = qApp->allWidgets();
	for (int i = 0; i < wl.size(); ++i)
	{
		QString clsname = wl.at(i)->metaObject()->className();
		if (clsname == kIDAMainWindowClassName)
		{
			if (mainWindow = qobject_cast<QMainWindow*>(wl.at(i)))
				return mainWindow;
		}
	}
#endif
	return nullptr;
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

