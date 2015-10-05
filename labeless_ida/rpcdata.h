/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include "types.h"
#include <QObject>

class RpcData : public QObject
{
	Q_OBJECT

public:
	enum State
	{
		ST_Working,
		ST_Received,
		ST_Done,
		ST_Failed
	};

	RpcData(uint64_t jobId_ = 0,
		uint32_t retryCount_ = 0,
		const std::string& script_ = std::string(),
		const std::string& scriptExternObj_ = std::string(),
		const std::string& params_ = std::string(),
		ICommandPtr iCmd_ = ICommandPtr(),
		RpcReadyToSendHandler readyToSendHandler_ = RpcReadyToSendHandler());


	void emitReceived();
	void emitParsed();
	void emitFailed(const QString& message);

	inline State state() { return m_State; }
signals:
	void received();
	void parsed();
	void failed(QString message);

public:
	uint64_t jobId;
	uint32_t retryCount;
	std::string script;
	std::string scriptExternObj;
	std::string params;
	std::shared_ptr<rpc::Response> response;
	ICommandPtr iCmd;
	State m_State;

	RpcReadyToSendHandler readyToSendHandler;
};

