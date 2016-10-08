/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "rpcdata.h"

RpcData::RpcData(uint64_t jobId_,
	uint32_t retryCount_,
	const std::string& script_,
	const std::string& scriptExternObj_,
	const std::string& params_,
	ICommandPtr iCmd_,
	RpcReadyToSendHandler readyToSendHandler_)
	: jobId(jobId_)
	, retryCount(retryCount_)
	, script(script_)
	, scriptExternObj(scriptExternObj_)
	, params(params_)
	, iCmd(iCmd_)
	, m_State(ST_Working)
	, readyToSendHandler(readyToSendHandler_)
{
}

void RpcData::emitReceived()
{
	m_State = ST_Received;
	emit received();
}

void RpcData::emitParsed()
{
	m_State = ST_Done;
	emit parsed();
}

void RpcData::emitFailed(const QString& message)
{
	m_State = ST_Failed;
	emit failed(message);
}
