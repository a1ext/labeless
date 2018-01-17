/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <memory>
#include <string>
#include <pro.h>

#include "../types.h"
#include "../jedi.h"

#include <QList>
#include <QPointer>

class RpcData;


struct ICommand
{
	uint64_t	base;
	uint64_t	remoteBase;
	uint64_t	jobId;
	bool		pending;

	std::string stdOut;
	std::string stdErr;
	std::string error;

	virtual bool serialize(QPointer<RpcData> rd) const = 0;
	virtual bool parseResponse(QPointer<RpcData> rd);

	ICommand()
		: base(0)
		, remoteBase(0)
		, jobId(0)
		, pending(false)
	{}
protected:
	template <typename T>
	std::string makeRpcParam(T* pMessage, int requestType) const throw ();
};

struct ExecPyScript : public ICommand
{
	struct Data
	{
		std::string idaScript;
		std::string idaExtern;
		std::string ollyScript;
		bool ollyResultIsSet;
		std::string ollyResult;

		Data()
			: ollyResultIsSet(false)
		{}
	};
	Data d;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct LabelsSync : public ICommand
{
	struct Data
	{
		uint64_t	ea;
		std::string label;
		Data(uint64_t ea_, const std::string& label_)
			: ea(ea_)
			, label(label_)
		{}
	};
	typedef QList<Data> DataList;

	DataList data;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct CommentsSync : public ICommand
{
	struct Data
	{
		uint64_t	ea;
		std::string comment;
		Data(uint64_t ea_, const std::string& comment_)
			: ea(ea_)
			, comment(comment_)
		{}
	};
	typedef QList<Data> DataList;

	DataList data;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct GetMemoryMapReq : public ICommand
{
	MemoryRegionList data;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct ReadMemoryRegions : public ICommand
{
	struct t_memory : public MemoryRegion
	{
		std::string raw;
		t_memory(uint64_t base_, uint64_t size_, uint32_t protect_, const std::string& raw_, bool forceProtect_)
			: MemoryRegion(base_, size_, protect_, forceProtect_)
			, raw(raw_)
		{}
	};
	typedef QList<t_memory> DataList;

	DataList data;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct AnalyzeExternalRefs : public ICommand
{
	struct Req
	{
		uint64_t	eaFrom;
		uint64_t	eaTo;
		int32_t		increment;
		uint64_t	base;
		uint64_t	size;

		Req()
			: eaFrom(0)
			, eaTo(0)
			, increment(0)
			, base(0)
			, size(0)
		{}
	};
	struct PointerData
	{
		uint64_t ea;
		std::string module;
		std::string procName;

		PointerData()
			: ea(0)
		{}
	};
	typedef QList<PointerData> Pointers;

	enum RefType
	{
		RT_Unknown = 0,
		RT_JmpConst,
		RT_ImmConst,
		RT_AddrConst
	};

	struct RefData
	{
		uint64_t instrEA;
		uint64_t len;
		RefType type;
		uint64_t val;
		std::string dis;
		std::string module;
		std::string proc;

		RefData()
			: instrEA(0)
			, len(0)
			, type(RT_Unknown)
			, val(0)
		{}
	};
	typedef QList<RefData> RefDataList;

	Req req;
	Pointers ptrs;
	RefDataList rdl;
	uint64_t rip;

	AnalyzeExternalRefs()
		: rip(0)
	{}

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct CheckPEHeaders : public ICommand
{
	// params
	uint64_t size;

	// result
	bool peValid;
	ExportItemList exports;
	SectionList sections;

	CheckPEHeaders()
		: size(0)
		, peValid(false)
	{}

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct GetBackendInfo : public ICommand
{
	GetBackendInfo()
		: bitness(0)
	{}

	uint32_t bitness;
	std::string dbg_name;
	std::string dbg_ver;
	std::string labeless_ver;


	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct AutoCompleteCode : public ICommand
{
	AutoCompleteCode()
		: zline(0)
		, zcol(0)
		, callSigsOnly(false)
		, jresult(new jedi::Result)
	{}

	// params
	std::string source;
	quint32 zline;
	quint32 zcol;
	bool callSigsOnly;

	// result
	QSharedPointer<jedi::Result> jresult;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};
