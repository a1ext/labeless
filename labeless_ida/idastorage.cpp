/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "idastorage.h"

// IDA
#include "types.h"
#include "externsegdata.h"

namespace {

enum LabelessDbStorageVersion
{
	LDSV_V0 = 0,
	LDSV_V1 = 1,
	LDSV_V2 = 2,

	// the latest
	LDSV_LATEST = LDSV_V2
};

enum ExternSegDataNodeAltType
{
	ESDNAT_LowStart		= 0,
	ESDNAT_LowLen		= 1,
	ESDNAT_LowUsed		= 2,
	ESDNAT_HighStart	= 3,
	ESDNAT_HighLen		= 4,
	ESDNAT_HighUsed		= 5,
	ESDNAT_ImpCount		= 6
};

enum ExternSegImpsItemAltType
{
	ESIIAT_Ordinal		= 0,
	ESIIAT_Module		= 0,
	ESIIAT_EA			= 1,
	ESIIAT_Proc			= 1,
	//ESIIAT_IndexHigh	= 2,
	ESIIAT_JoinedName	= 2
};

enum SettingsNodeAltType
{
	LNAT_Port = 0,
	LNAT_RmoteModBase,
	LNAT_Demangle,
	LNAT_LocalLabels,
	LNAT_CommentsSync,
	LNAT_RemoveFuncArgs
};

enum StorageVersionAltType
{
	SVAT_Version = 0
};


static const std::string kNetNodeLabeless = "$ labeless";
static const std::string kNetNodeExternSegData = "$ externsegdata";
static const std::string kNetNodeExternSegImps = "$ externsegimps";
static const std::string kNetNodeStorageVer = "$ labelessver";

LabelessDbStorageVersion getStorageVersion()
{
	netnode n;
	n.create(kNetNodeStorageVer.c_str());
	return static_cast<LabelessDbStorageVersion>(n.altval(SVAT_Version));
}

/*void externSegDataLowFillLowSegmentInfo(ExternSegData& result)
{
	netnode n;
	n.create(kNetNodeExternSegData.c_str());

	result.start = n.altval(ESDNAT_LowStart);
	result.len = n.altval(ESDNAT_LowLen);
	result.used = n.altval(ESDNAT_LowUsed);
}

bool externSegDataLoadV0(ExternSegData& result)
{
	externSegDataLowFillLowSegmentInfo(result);

	netnode n;
	n.create(kNetNodeExternSegImps.c_str());

	char buff[MAXSTR] = {};

	for (uint64_t i = 0, e = result.used * 3; i < e; i += 3)
	{
		ImportEntry ie;
		ie.index = i / 3;
		ie.ordinal = n.altval(i + ESIIAT_Ordinal);
		if (n.supstr(i + ESIIAT_Module, buff, MAXSTR) > 0)
			ie.module = buff;
		if (n.supstr(i + ESIIAT_Proc, buff, MAXSTR) > 0)
			ie.proc = buff;

		std::string key;
		if (n.supstr(i + ESIIAT_JoinedName, buff, MAXSTR) > 0)
			key = buff;

		if (!ie.module.empty() && (!ie.proc.empty() || ie.ordinal) && !key.empty())
			result.imports[key] = ie;
	}
	return true;
}

bool externSegDataLoadV1(ExternSegData& result)
{
	externSegDataLowFillLowSegmentInfo(result);

	netnode nHigh;
	nHigh.create(kNetNodeExternSegData.c_str());

	result.startHigh = nHigh.altval(ESDNAT_HighStart);
	result.lenHigh = nHigh.altval(ESDNAT_HighLen);
	result.usedHigh = nHigh.altval(ESDNAT_HighUsed);

	netnode n;
	n.create(kNetNodeExternSegImps.c_str());

	char buff[MAXSTR] = {};
	uint64_t totalItems = result.used + result.usedHigh;

	for (uint64_t i = 0, e = totalItems * 3; i < e; i += 3)
	{
		ImportEntry ie;
		ie.ordinal = n.altval(i + ESIIAT_Ordinal);
		ie.index = n.altval(i + ESIIAT_Index);
		ie.indexHigh = n.altval(i + ESIIAT_IndexHigh);

		if (n.supstr(i + ESIIAT_Module, buff, MAXSTR) > 0)
			ie.module = buff;
		if (n.supstr(i + ESIIAT_Proc, buff, MAXSTR) > 0)
			ie.proc = buff;

		std::string key;
		if (n.supstr(i + ESIIAT_JoinedName, buff, MAXSTR) > 0)
			key = buff;

		if (!ie.module.empty() && (!ie.proc.empty() || ie.ordinal) && !key.empty())
			result.imports[key] = ie;
		else
			msg("%s: error, Invalid externSegData entry, index: %llu\n", __FUNCTION__, i);
	}

	return true;
}*/

bool externSegDataLoadV2(ExternSegData& result)
{
	netnode nCommon;
	nCommon.create(kNetNodeExternSegData.c_str());

	const auto impCount = nCommon.altval(ESDNAT_ImpCount);

	netnode n;
	n.create(kNetNodeExternSegImps.c_str());
	char buff[MAXSTR] = {};

	for (uint64_t i = 0; i < impCount; ++i)
	{
		const auto idx = i * 3;
		ImportEntry ie;
		ie.ordinal = n.altval(idx + ESIIAT_Ordinal);
		ie.ea = n.altval(idx + ESIIAT_EA);

		if (n.supstr(idx + ESIIAT_Module, buff, MAXSTR) > 0)
			ie.module = buff;
		if (n.supstr(idx + ESIIAT_Proc, buff, MAXSTR) > 0)
			ie.proc = buff;

		std::string key;
		if (n.supstr(idx + ESIIAT_JoinedName, buff, MAXSTR) > 0)
			key = buff;

		if (ie.isValid() && !key.empty())
			result.imports[key] = ie;
		else
			msg("%s: error, Invalid externSegData entry, index: %llu\n", __FUNCTION__, i);
	}
	return true;
}

} // anonymous


namespace storage {

bool loadExternSegData(ExternSegData& result)
{
	const auto storageVer = getStorageVersion();

	switch (storageVer)
	{
	case LDSV_V0:
		//return externSegDataLoadV0(result);
		break;
	case LDSV_V1:
		//return externSegDataLoadV1(result);
		break;
	case LDSV_V2:
		return externSegDataLoadV2(result);
	}
	return false;
}

bool storeExternSegData(const ExternSegData& value)
{
	netnode n;
	n.create(kNetNodeExternSegData.c_str());
	/*n.altset(ESDNAT_LowStart, value.start);
	n.altset(ESDNAT_LowLen, value.len);
	n.altset(ESDNAT_LowUsed, value.imports.size());*/
	n.altset(ESDNAT_ImpCount, value.imports.size());

	// { v1
	/*n.altset(ESDNAT_HighStart, value.startHigh);
	n.altset(ESDNAT_HighLen, value.lenHigh);
	n.altset(ESDNAT_HighUsed, value.usedHigh);*/
	// }

	netnode n2;
	n2.create(kNetNodeExternSegImps.c_str());

	uint64_t idx = 0;
	for (auto it = value.imports.cbegin(), end = value.imports.cend(); it != end; ++it, idx += 3)
	{
		const ImportEntry& ie = it->second;
		n2.supset(idx + ESIIAT_Module, ie.module.c_str());
		n2.supset(idx + ESIIAT_Proc, ie.proc.c_str());
		n2.supset(idx + ESIIAT_JoinedName, it->first.c_str());
		n2.altset(idx + ESIIAT_Ordinal, it->second.ordinal);
		n2.altset(idx + ESIIAT_EA, ie.ea);
	}

	netnode nver;
	nver.create(kNetNodeStorageVer.c_str());
	nver.altset(SVAT_Version, LDSV_LATEST);
	return true;
}

bool loadDbSettings(Settings& result)
{
	netnode n;
	const bool nodeExists = !n.create(kNetNodeLabeless.c_str());
	if (!nodeExists)
		return false;

	char buff[MAXSTR] = {};
	if (n.supstr(0, buff, MAXSTR) >= 2)
		result.host = buff;
	result.port = n.altval(LNAT_Port);
	result.remoteModBase = n.altval(LNAT_RmoteModBase);
	result.demangle = n.altval(LNAT_Demangle) != 0;
	result.localLabels = n.altval(LNAT_LocalLabels) != 0;
	result.commentsSync = static_cast<Settings::CommentsSync>(n.altval(LNAT_CommentsSync));
	result.removeFuncArgs = n.altval(LNAT_RemoveFuncArgs) != 0;
	return true;
}

bool storeDbSettings(const Settings& result)
{
	netnode n;
	n.create(kNetNodeLabeless.c_str());

	n.supset(0, result.host.c_str());
	n.altset(LNAT_Port, result.port);
	n.altset(LNAT_RmoteModBase, result.remoteModBase);
	n.altset(LNAT_Demangle, result.demangle ? 1 : 0);
	n.altset(LNAT_LocalLabels, result.localLabels ? 1 : 0);
	n.altset(LNAT_CommentsSync, result.commentsSync);
	n.altset(LNAT_RemoveFuncArgs, result.removeFuncArgs ? 1 : 0);
	return true;
}

} // storage
