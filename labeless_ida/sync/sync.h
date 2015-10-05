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

#include <QList>
#include <QPointer>

class RpcData;


struct ICommand
{
	ea_t		base;
	ea_t		remoteBase;
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
	};
	Data d;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct FuncNameSync : public ICommand
{
	struct Data
	{
		uint32_t	ea;
		std::string label;
		Data(uint32_t ea_, const std::string& label_)
			: ea(ea_)
			, label(label_)
		{}
	};
	typedef QList<Data> DataList;

	DataList data;

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct LocalLabelsSync : public ICommand
{
	struct Data
	{
		uint32_t	ea;
		std::string label;
		Data(uint32_t ea_, const std::string& label_)
			: ea(ea_)
			, label(label_)
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
		t_memory(ea_t base_, uint32_t size_, uint32_t protect_, const std::string& raw_)
			: MemoryRegion(base_, size_, protect_)
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
		ea_t		eaFrom;
		ea_t		eaTo;
		int32_t		increment;
		ea_t		base;
		uint32_t	size;

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
		ea_t ea;
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
		ea_t instrEA;
		uint32_t len;
		RefType type;
		uint32_t val;
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
	ea_t eip;

	AnalyzeExternalRefs()
		: eip(0)
	{}

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};

struct CheckPEHeaders : public ICommand
{
	// params
	ea_t base;
	uint32_t size;

	// result
	bool peValid;
	ExportItemList exports;
	SectionList sections;

	CheckPEHeaders()
		: base(0)
		, size(0)
		, peValid(false)
	{}

	virtual bool serialize(QPointer<RpcData> rd) const override;
	virtual bool parseResponse(QPointer<RpcData> rd) override;
};
