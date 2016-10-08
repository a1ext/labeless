/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include "types.h"

struct IDADump
{
	enum State
	{
		ST_None,
		ST_CheckingPEHeader,
		ST_CheckingPEHeaderFinished,
		ST_ReadingMemoryRegions,
		ST_ReadingMemoryRegionsFinished,
		ST_AnalyzingExternalRefs,
		ST_PostAnalysis,
		ST_Done
	};

	struct AnalyseExtRefsWrapper
	{
		RpcDataPtr rpcData;
		bool finished;

		AnalyseExtRefsWrapper(RpcDataPtr rpcData_)
			: rpcData(rpcData_)
			, finished(false)
		{}
	};

public:
	State state;
	bool wipe;
	bool analyzePEHeader;

	ExportItemList exports;
	SectionList sections;

	std::shared_ptr<ReadMemoryRegions> readMemRegions;
	std::shared_ptr<CheckPEHeaders> checkPEHeaders;
	QList<AnalyseExtRefsWrapper> analyzeExtRefs;

	IDADump& nextState(RpcDataPtr rpcData);

private:
	inline bool nopeHandler(RpcDataPtr){ return true; }
	bool onCheckingPEHeaderState(RpcDataPtr rpcData);
	bool onAnalyzeExternalRefsState(RpcDataPtr rpcData);

public:
	IDADump();
	~IDADump();
};

