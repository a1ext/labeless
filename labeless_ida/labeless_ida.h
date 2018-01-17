/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include <memory>
#include <set>
#include <string>

#include <QAtomicInt>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QPointer>
#include <QSharedPointer>
#include <QThread>
#include <QWaitCondition>

#include "compat.h"
#include "externsegdata.h"
#include "sync/sync.h"
#include "rpcdata.h"
#include "idadump.h"


// fwd
QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QMainWindow)
struct form_actions_t;

class PyOllyView;

namespace jedi {
struct State;
struct Result;
struct Request;
} // jedi


#if (IDA_SDK_VERSION < 700)
typedef int hook_cb_t_ret_type_t;
#else // IDA_SDK_VERSION < 700
typedef ::ssize_t hook_cb_t_ret_type_t;
#endif // IDA_SDK_VERSION < 700


class Labeless : public QObject
{
	Q_OBJECT

	Labeless();
	~Labeless();
	Q_DISABLE_COPY(Labeless);
public:
	enum HelperMsg
	{
		HM_ResultReady,
		HM_Log
	};
	static Labeless& instance();

	void setTargetHostAddr(const std::string& ip, quint16 port);

	bool isEnabled() const { return m_Enabled; }
	bool setEnabled();
	void enableMenuActions(bool enabled);

	bool firstInit();
	bool initialize();
	void terminate();
	void shutdown();

	std::string ollyHost() const;
	quint16 ollyPort() const;
	ea_t remoteModuleBase();

	/**< hook processor_t::idp_notify - begin */
	void onRename(uint64_t ea, const std::string& newName);
	void onAutoanalysisFinished();
	void onMakeCode(ea_t ea, ::asize_t size);
	void onMakeData(ea_t ea, ::flags_t flags, ::tid_t tid, ::asize_t len);
	void onAddCref(ea_t from, ea_t to, cref_t type);
	void onAddDref(ea_t from, ea_t to, dref_t type);
	/**< hook processor_t::idp_notify - end */

	void addLabelsSyncData(const LabelsSync::DataList& sds);
	void addCommentsSyncData(const CommentsSync::DataList& dl);
	RpcDataPtr addRpcData(ICommandPtr cmd,
		RpcReadyToSendHandler ready = [](RpcDataPtr){ return true; },
		const QObject* receiver = nullptr, const char* member = nullptr,
		Qt::ConnectionType ct = Qt::QueuedConnection);
	RpcDataPtr addRpcData(RpcDataPtr rpc,
		const QObject* receiver = nullptr, const char* member = nullptr,
		Qt::ConnectionType ct = Qt::QueuedConnection);

	void setRemoteModuleBase(ea_t base);

	bool importCode(bool wipe);

	inline bool isShowAllResponsesInLog() const { return m_ShowAllResponsesInLog; }

	static bool testConnect(const std::string& host, uint16_t port, QString& errorMsg);

	static hook_cb_t_ret_type_t idaapi ui_callback(void* /*user_data*/, int notification_code, va_list va);
	static hook_cb_t_ret_type_t idaapi idp_callback(void* /*user_data*/, int notification_code, va_list va);
	static hook_cb_t_ret_type_t idaapi idb_callback(void* /*user_data*/, int notification_code, va_list va);

	inline const Settings& settings() const { return m_Settings; }
	inline bool pauseNotificationHanlingEnabled() const { return m_PauseNotificationHandlingEnabled; }
	inline WORD pauseNotificationPort() const { return m_PauseNotificationPort; }

private slots:
	void onRunPythonScriptFinished();
	void onGetBackendInfoFinished();
	void onGetMemoryMapFinished();
	void onCheckPEHeadersFinished();
	void onReadMemoryRegionsFinished();
	void onAnalyzeExternalRefsFinished();
	void onAutoCompleteRemoteFinished();

	void onSyncResultReady();
	void onRpcRequestFailed(QString message);

	void onPyOllyFormClose();
	void onRunScriptRequested();
	inline void setShowAllResponsesInLog(bool show) { m_ShowAllResponsesInLog = show; }
	void onLogAnchorClicked(const QString& value);
	void onClearLogsRequested();

public slots:
	void onSyncronizeAllRequested();
	void onSettingsRequested();
	void onWipeAndImportRequested();
	void onKeepAndImportRequested();
	void onLoadStubDBRequested();
	void onShowRemotePythonExecutionViewRequested();
	void onTestConnectRequested();
	void onLogMessage(const QString& message, const QString& prefix);
	void onTestConnectFinished(bool ok, const QString& error);
	void onAutoCompletionFinished();
	void onAutoCompleteRequested(QSharedPointer<jedi::Request> r);
	void onAutoCompleteRemoteRequested(QSharedPointer<jedi::Request> r);
	void onPauseNotificationReceived(void*);
	void onTogglePauseNotificationHandling(bool enabled);

private:
	static SOCKET connectToHost(const std::string& host, uint16_t port, QString& errorMsg, bool keepAlive = true, quint32 recvtimeout = 30 * 60 * 1000);

	void enableRunScriptButton(bool enabled);

	void openPythonEditorForm(int options = 0);
	void addLogItem(LogItem& logItem);

	size_t loadImportTable();
	void storeImportTable();
	bool createImportSegments(const std::map<uint64_t, AnalyzeExternalRefs::PointerData>& impData);
	void updateImportsNode();
	qstring getNewNameOfEntry() const;

	bool initIDAPython();
	bool initPython();

	Settings loadSettings();
	void storeSettings();
	bool isDbgBackendNotificatiosAllowed(const std::string& backendId);

private:
	bool addAPIEnumValue(const std::string& name, uval_t value);
	void addAPIConst(const AnalyzeExternalRefs::PointerData& pd);
	bool mergeMemoryRegion(IDADump& icInfo, const ReadMemoryRegions::t_memory& m, ea_t region_base, uint64_t region_size);
	//segment_t* getFirstOverlappedSegment(const area_t& area, segment_t* exceptThisSegment);
	bool createSegment(const compat::IDARange& area, uchar perm, uchar type, const std::string& data, segment_t& result);
	void getRegionPermissionsAndType(const IDADump& icInfo, const ReadMemoryRegions::t_memory& m, uchar& perm, uchar& type) const;
	bool askForSnapshotBeforeOverwrite(const compat::IDARange* area, const segment_t* seg, bool& snapshotTaken);

	bool make_dword_ptr(ea_t ea, asize_t size);
public:
	QAtomicInt						m_Enabled;
	bool							m_Initialized;
	QAtomicInt						m_IgnoreMakeData;
	QAtomicInt						m_IgnoreMakeCode;
	QAtomicInt						m_SuppressMessageBoxesFromIDA;

private:
	friend class RpcThreadWorker;
	friend class JediCompletionWorker;

	// settings
	mutable QMutex					m_ConfigLock;
	Settings						m_Settings;
	bool							m_SynchronizeAllNow;
	size_t							m_LabelSyncOnRenameIfZero;
	bool							m_ShowAllResponsesInLog;

	// RPC network worker vars
	QMutex							m_ThreadLock;
	QPointer<QThread>				m_Thread;
	QMutex							m_QueueLock;
	qlist<RpcDataPtr>				m_Queue;

	QWaitCondition					m_QueueCond;

	// GUI
	static compat::TWidget*			m_EditorTForm;
	QPointer<PyOllyView>			m_PyOllyView;

	qlist<segment_t>				m_CreatedSegments;
	ExternSegData					m_ExternSegData;
	ExternRefDataMap				m_ExternRefsMap;
	QList<IDADump>					m_DumpList;
	QMap<uint64_t, LogItem>			m_LogItems;
	QList<QAction*>					m_MenuActions;
	QAction*						m_PauseNotificationMenuAction;

	// auto-completion vars
	mutable QMutex					m_AutoCompletionLock;
	QMutex							m_AutoCompletionThreadLock;
	QPointer<QThread>				m_AutoCompletionThread;
	QSharedPointer<jedi::Request>	m_AutoCompletionRequest;
	QSharedPointer<jedi::Result>	m_AutoCompletionResult;
	QSharedPointer<jedi::State>		m_AutoCompletionState;
	QWaitCondition					m_AutoCompletionCond;

	// pause notification stuff
	bool							m_PauseNotificationHandlingEnabled;
	QPointer<QThread>				m_PauseNotificationThread;
	QMutex							m_PauseNotificationThreadLock;
	ea_t							m_PauseNotificationCursor;
	WORD							m_PauseNotificationPort;
	std::set<std::string>			m_PauseNotificationAllowedClients;
};
