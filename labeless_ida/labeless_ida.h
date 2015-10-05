/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include <memory>
#include <string>

#include <QAtomicInt>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QPointer>
#include <QSharedPointer>
#include <QThread>
#include <QWaitCondition>

#include <pro.h>
#include "sync/sync.h"
#include "rpcdata.h"
#include "idadump.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QMainWindow)
struct form_actions_t;

class PyOllyView;

class Labeless : public QObject
{
	Q_OBJECT

	Labeless();
	~Labeless();
	Labeless(const Labeless&) {};
	Labeless& operator=(const Labeless&) {};
public:
	enum HelperMsg
	{
		HM_ResultReady,
		HM_Log
	};
	static Labeless& instance();

	void setTargetHostAddr(const std::string& ip, WORD port);

	bool isEnabled() const { return m_Enabled; }
	bool setEnabled();
	void enableMenuActions(bool enabled);

	bool firstInit();
	bool initialize();
	void terminate();
	void shutdown();

	std::string ollyHost() const;
	WORD ollyPort() const;
	ea_t remoteModuleBase();

	/**< hook processor_t::idp_notify - begin */
	void onRename(uint32_t ea, const std::string& newName);
	void onAutoanalysisFinished();
	void onMakeCode(ea_t ea, ::asize_t size);
	void onMakeData(ea_t ea, ::flags_t flags, ::tid_t tid, ::asize_t len);
	void onAddCref(ea_t from, ea_t to, cref_t type);
	void onAddDref(ea_t from, ea_t to, dref_t type);
	/**< hook processor_t::idp_notify - end */

	void addFuncNameSyncData(const FuncNameSync::DataList& sds);
	void addLocLabelSyncData(const LocalLabelsSync::DataList& dl);
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

	static int idaapi ui_callback(void* /*user_data*/, int notification_code, va_list va);
	static int idaapi idp_callback(void* /*user_data*/, int notification_code, va_list va);

private slots:
	void onRunPythonScriptFinished();
	void onGetMemoryMapFinished();
	void onCheckPEHeadersFinished();
	void onReadMemoryRegionsFinished();
	void onAnalyzeExternalRefsFinished();

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

private:
	static SOCKET connectToHost(const std::string& host, uint16_t port, QString& errorMsg, bool keepAlive = true);

	static bool testConnect(const std::string& host, uint16_t port, QString& errorMsg);

	void enableRunScriptButton(bool enabled);

	void openPythonEditorForm(int options = 0);
	void addLogItem(LogItem& logItem);

	uint32_t loadImportTable();
	void storeImportTable();
	bool createImportSegment(ea_t from, ea_t to);
	void updateImportsNode();
	qstring getNewNameOfEntry() const;

	bool initIDAPython();
	bool runIDAPythonScript(const std::string& script, std::string& externObj, std::string& error);

	Settings loadSettings();
	void storeSettings();

	QMainWindow* findIDAMainWindow() const;
private:
	bool addAPIEnumValue(const std::string& name, uval_t value);
	bool mergeMemoryRegion(IDADump& icInfo, const ReadMemoryRegions::t_memory& m, ea_t region_base, uint32_t region_size);
	segment_t* getFirstOverlappedSegment(const area_t& area, segment_t* exceptThisSegment);
	bool createSegment(const area_t& area, uchar perm, uchar type, const std::string& data, segment_t& result);
	void getRegionPermissionsAndType(const IDADump& icInfo, const ReadMemoryRegions::t_memory& m, uchar& perm, uchar& type) const;

	bool make_dword(ea_t ea, asize_t size);
public:
	QAtomicInt						m_Enabled;
	bool							m_Initialized;
	QAtomicInt						m_IgnoreMakeData;
	QAtomicInt						m_IgnoreMakeCode;
	QAtomicInt						m_SuppressMessageBoxesFromIDA;

private:
	friend class RpcThreadWorker;

	mutable QMutex					m_ConfigLock;
	Settings						m_Settings;
	bool							m_SynchronizeAllNow;
	size_t							m_LabelSyncOnRenameIfZero;
	bool							m_ShowAllResponsesInLog;

	QMutex							m_ThreadLock;
	QPointer<QThread>				m_Thread;
	QMutex							m_QueueLock;
	qlist<RpcDataPtr>				m_Queue;

	QWaitCondition					m_QueueCond;

	static TForm*					m_EditorTForm;
	QPointer<PyOllyView>			m_PyOllyView;

	qlist<segment_t>				m_CreatedSegments;
	ExternSegData					m_ExternSegData;
	ExternRefDataMap				m_ExternRefsMap;
	QList<IDADump>					m_DumpList;
	QMap<uint64_t, LogItem>			m_LogItems;
	QList<QAction*>					m_MenuActions;
};
