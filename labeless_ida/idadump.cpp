/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "idadump.h"

#include <functional>

#include "rpcdata.h"


IDADump::IDADump()
	: state(ST_None)
	, wipe(false)
	, analyzePEHeader(true)
{
}

IDADump::~IDADump()
{
}

IDADump& IDADump::nextState(RpcDataPtr rpcData)
{
	typedef bool (IDADump::* StateHandler_t)(RpcDataPtr);
	static const struct {
		State st;
		State next;
		StateHandler_t stateHandler;
	} kStateList[] = {
		{ ST_None,							ST_CheckingPEHeader,				&IDADump::onCheckingPEHeaderState },
		{ ST_CheckingPEHeader,				ST_CheckingPEHeaderFinished,		&IDADump::nopeHandler },
		{ ST_CheckingPEHeaderFinished,		ST_ReadingMemoryRegions,			&IDADump::nopeHandler },
		{ ST_ReadingMemoryRegions,			ST_ReadingMemoryRegionsFinished,	&IDADump::nopeHandler },
		{ ST_ReadingMemoryRegionsFinished,	ST_AnalyzingExternalRefs,			&IDADump::nopeHandler },
		{ ST_AnalyzingExternalRefs,			ST_PostAnalysis,					&IDADump::onAnalyzeExternalRefsState },
		{ ST_PostAnalysis,					ST_Done,							&IDADump::nopeHandler }
	};

	bool ok = false;
	for (unsigned i = 0; i < _countof(kStateList); ++i)
	{
		if (kStateList[i].st == state)
		{
			if ((this->*kStateList[i].stateHandler)(rpcData))
			{
				msg("%s: changing state from %u to %u\n", __FUNCTION__, state, kStateList[i].next);
				state = kStateList[i].next;
				if (state == ST_Done)
					msg("%s: done\n", __FUNCTION__);
			}
			else
			{
				msg("%s: keeping state at %u\n", __FUNCTION__, state);
			}
			ok = true;
			break;
		}
	}
	if (!ok)
		msg("%s: Invalid state: %u, unable to go next\n", __FUNCTION__, state);
	return *this;
}

bool IDADump::onCheckingPEHeaderState(RpcDataPtr rpcData)
{
	if (!analyzePEHeader)
	{
		state = ST_ReadingMemoryRegions;
		return false;
	}
	return true;
}

bool IDADump::onAnalyzeExternalRefsState(RpcDataPtr rpcData)
{
	bool allFinished = true;
	for (int i = 0; i < analyzeExtRefs.count(); ++i)
	{
		auto& p = analyzeExtRefs[i];
		if (p.rpcData == rpcData)
			p.finished = true;
		if (!p.finished)
			allFinished = false;
	}
	return allFinished;
}
