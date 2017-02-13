/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "sync.h"
#include <sstream>
#include <QFile>

#include "../util/util_protobuf.h"
#include "../rpcdata.h"
#include "../../common/cpp/rpc.pb.h"



bool ICommand::parseResponse(QPointer<RpcData> rd)
{
	if (!rd || !rd->response || !rd->response->IsInitialized())
		return false;

	try
	{
		stdOut = rd->response->std_out();
		stdErr = rd->response->std_err();
		if (rd->response->has_job_id())
		{
			jobId = rd->response->job_id();
			if (!rd->jobId)
				rd->jobId = jobId;
		}
		if (rd->response->has_job_status())
			pending = rd->response->job_status() == rpc::Response::JS_PENDING;

		error = rd->response->error();
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
		return false;
	}
	return true;
}


template <typename T>
std::string ICommand::makeRpcParam(T* pMessage, int requestType) const throw ()
{
	rpc::RpcRequest rpcRequest;
	rpcRequest.set_request_type(static_cast<rpc::RpcRequest_RequestType>(requestType));
	if (pMessage)
	{
		switch (requestType)
		{
		case rpc::RpcRequest::RPCT_UNKNOWN:
			msg("%s: Invalid RpcRequest type passed: %08X\n", requestType);
			return {};
		case rpc::RpcRequest::RPCT_MAKE_NAMES:
			rpcRequest.mutable_make_names_req()->CopyFrom(*pMessage);
			break;
		case rpc::RpcRequest::RPCT_MAKE_COMMENTS:
			rpcRequest.mutable_make_comments_req()->CopyFrom(*pMessage);
			break;
		case rpc::RpcRequest::RPCT_GET_MEMORY_MAP:
			break;
		case rpc::RpcRequest::RPCT_READ_MEMORY_REGIONS:
			rpcRequest.mutable_read_memory_regions_req()->CopyFrom(*pMessage);
			break;
		case rpc::RpcRequest::RPCT_ANALYZE_EXTERNAL_REFS:
			rpcRequest.mutable_analyze_external_refs_req()->CopyFrom(*pMessage);
			break;
		case rpc::RpcRequest::RPCT_CHECK_PE_HEADERS:
			rpcRequest.mutable_check_pe_headers_req()->CopyFrom(*pMessage);
			break;
		}
	}

	return rpcRequest.SerializeAsString();
}

bool ExecPyScript::serialize(QPointer<RpcData> rd) const
{
	rd->script = d.ollyScript;
	rd->scriptExternObj = d.idaExtern;
	return true;
}

bool ExecPyScript::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;

	try
	{
		if (rd->response->has_script_result_obj())
		{
			d.ollyResult = rd->response->script_result_obj();
			d.ollyResultIsSet = true;
		}
		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse ExecPyScript response\n", __FUNCTION__);
	}
	return false;
}

bool LabelsSync::serialize(QPointer<RpcData> rd) const
{
	try
	{
		rpc::RpcRequest rpcRequest;
		rpcRequest.set_request_type(rpc::RpcRequest::RPCT_MAKE_NAMES); 
		rpc::MakeNamesRequest* const request = rpcRequest.mutable_make_names_req();
		request->set_base(base);
		request->set_remote_base(remoteBase);

		for (auto it = data.begin(), end = data.end(); it != end; ++it)
		{
			const Data& sd = *it;
			auto v = request->add_names();
			v->set_ea(sd.ea);
			v->set_name(sd.label);
		}
		rd->script.clear();
		rd->params = rpcRequest.SerializeAsString();

		return true;
	}
	catch (...)
	{
		msg("%s: Unable to serialize rpc::MakeNamesRequest\n", __FUNCTION__);
	}
	return false;
}

bool LabelsSync::parseResponse(QPointer<RpcData> rd)
{
	return ICommand::parseResponse(rd);
}

bool CommentsSync::serialize(QPointer<RpcData> rd) const
{
	try
	{
		rpc::RpcRequest rpcRequest;
		rpcRequest.set_request_type(rpc::RpcRequest::RPCT_MAKE_COMMENTS);

		rpc::MakeCommentsRequest* const request = rpcRequest.mutable_make_comments_req();
		request->set_base(base);
		request->set_remote_base(remoteBase);

		for (auto it = data.begin(), end = data.end(); it != end; ++it)
		{
			const Data& sd = *it;
			auto v = request->add_names();
			v->set_ea(sd.ea);
			v->set_name(sd.comment);
		}
		rd->script.clear();
		rd->params = rpcRequest.SerializeAsString();
		return true;
	}
	catch (...)
	{
		msg("%s: Unable to serialize rpc::MakeCommentsRequest\n", __FUNCTION__);
	}
	return false;
}

bool CommentsSync::parseResponse(QPointer<RpcData> rd)
{
	return ICommand::parseResponse(rd);
}

bool GetMemoryMapReq::serialize(QPointer<RpcData> rd) const
{
	rd->script.clear();
	rpc::RpcRequest rpcRequest;
	rpcRequest.set_request_type(rpc::RpcRequest::RPCT_GET_MEMORY_MAP);
	rd->params = rpcRequest.SerializeAsString();
	return true;
}

bool GetMemoryMapReq::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;
	try
	{
		rpc::GetMemoryMapResult result;
		if (!util::protobuf::parseBigMessage(result, rd->response->rpc_result()))
		{
			msg("%s: util::protobuf::parseBigMessage() failed\n", __FUNCTION__);
			return false;
		}

		data.clear();
		for (auto it = result.memories().begin(); it != result.memories().end(); ++it)
		{
			MemoryRegion m(it->base(), it->size(), it->access());
			m.name = it->name();

			data.push_back(m);
		}
		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse GetMemoryMapReq response\n", __FUNCTION__);
	}
	return false;
}

bool ReadMemoryRegions::serialize(QPointer<RpcData> rd) const
{
	try
	{
		rpc::RpcRequest rpcRequest;
		rpcRequest.set_request_type(rpc::RpcRequest::RPCT_READ_MEMORY_REGIONS);

		rpc::ReadMemoryRegionsRequest* const request = rpcRequest.mutable_read_memory_regions_req();
		for (auto it = data.begin(), end = data.end(); it != end; ++it)
		{
			rpc::ReadMemoryRegionsRequest_Region* const region = request->add_regions();
			region->set_addr(it->base);
			region->set_size(it->size);
		}

		rd->script.clear();
		rd->params = rpcRequest.SerializeAsString();
		return true;
	}
	catch (...)
	{
		msg("%s: Unable to serialize rpc::ReadMemoryRegionsRequest\n", __FUNCTION__);
	}
	return false;
}

bool ReadMemoryRegions::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;

	try
	{
		rpc::ReadMemoryRegionsResult result;
		if (!util::protobuf::parseBigMessage(result, rd->response->rpc_result()))
		{
			msg("%s:util::protobuf::parseBigMessage() failed\n", __FUNCTION__);
			return false;
		}

		for (int i = 0, e = result.memories_size(); i < e; ++i)
		{
			const auto& memory = result.memories().Get(i);
			uint64_t addr = memory.addr();
			uint64_t size = memory.size();

			if (data.size() < i)
				continue;
			t_memory& m = data[i];
			if (m.base != addr || m.size != size)
			{
				msg("%s: data inconsistence, addr/size mismatch\n", __FUNCTION__);
				return false;
			}
			m.raw = memory.mem();
			if (!m.protect)
				m.protect = memory.protect();
		}
		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse ReadMemoryRegions response\n", __FUNCTION__);
	}
	return false;
}

bool AnalyzeExternalRefs::serialize(QPointer<RpcData> rd) const
{
	try
	{
		rpc::RpcRequest rpcRequest;
		rpcRequest.set_request_type(rpc::RpcRequest::RPCT_ANALYZE_EXTERNAL_REFS);

		rpc::AnalyzeExternalRefsRequest* const request = rpcRequest.mutable_analyze_external_refs_req();
		request->set_ea_from(req.eaFrom);
		request->set_ea_to(req.eaTo);
		request->set_increment(req.increment);
		request->set_analysing_base(req.base);
		request->set_analysing_size(req.size);

		rd->script.clear();
		rd->params = rpcRequest.SerializeAsString();
		return true;
	}
	catch (...)
	{
		msg("%s: Unable to serialize rpc::AnalyzeExternalRefsRequest\n", __FUNCTION__);
	}
	return false;
}

bool AnalyzeExternalRefs::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;

	try
	{
		rpc::AnalyzeExternalRefsResult result;
		if (!util::protobuf::parseBigMessage(result, rd->response->rpc_result()))
		{
			msg("%s: util::protobuf::parseBigMessage() failed\n", __FUNCTION__);
			return false;
		}

		rip = result.context().rip();

		const auto& api_constants = result.api_constants();
		for (auto it = api_constants.begin(), end = api_constants.end(); it != end; ++it)
		{
			const auto& v = *it;
			PointerData pd;
			pd.ea		= v.ea();
			pd.module	= v.module();
			pd.procName = v.proc();

			if (!pd.ea || pd.module.empty() || pd.procName.empty())
				continue;
			ptrs.push_back(pd);
		}

		const auto& refs = result.refs();
		for (auto it = refs.begin(), end = refs.end(); it != end; ++it)
		{
			const auto& v = *it;
			RefData rd;
			rd.instrEA	= v.ea();
			rd.len		= v.len();
			rd.dis		= v.dis();
			rd.val		= v.v();
			rd.type		= static_cast<RefType>(v.ref_type());
			rd.module	= v.module();
			rd.proc		= v.proc();

			if (rd.instrEA && rd.val && !rd.dis.empty() && rd.len > 0 && !rd.module.empty() && !rd.proc.empty())
				rdl.push_back(rd);
		}
		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse AnalyzeExternalRefs response\n", __FUNCTION__);
	}
	return false;
}

bool CheckPEHeaders::serialize(QPointer<RpcData> rd) const
{
	try
	{
		rpc::RpcRequest rpcRequest;
		rpcRequest.set_request_type(rpc::RpcRequest::RPCT_CHECK_PE_HEADERS);

		rpc::CheckPEHeadersRequest* const request = rpcRequest.mutable_check_pe_headers_req();
		request->set_base(base);
		request->set_size(size);

		rd->script.clear();
		rd->params = rpcRequest.SerializeAsString();
		return true;
	}
	catch (...)
	{
		msg("%s: Unable to serialize rpc::CheckPEHeadersRequest\n", __FUNCTION__);
	}
	return false;
}

bool CheckPEHeaders::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;

	try
	{
		rpc::CheckPEHeadersResult result;
		if (!util::protobuf::parseBigMessage(result, rd->response->rpc_result()))
		{
			msg("%s: util::protobuf::parseBigMessage() failed\n", __FUNCTION__);
			return false;
		}

		peValid = result.pe_valid();
		if (peValid)
		{
			const auto& exps = result.exps();
			for (auto it = exps.begin(), end = exps.end(); it != end; ++it)
			{
				ExportItem item = {
					it->ea(),
					it->ord(),
					it->name()
				};
				exports.append(item);
			}
			const auto& sects = result.sections();
			for (auto it = sects.begin(), end = sects.end(); it != end; ++it)
			{
				Section s = {
					it->name(),
					it->va(),
					it->v_size(),
					it->raw(),
					it->raw_size(),
					it->characteristics()
				};
				sections.append(s);
			}
		}
		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse CheckPEHeadersRequest response\n", __FUNCTION__);
	}
	return false;
}

bool GetBackendInfo::serialize(QPointer<RpcData> rd) const
{
	rd->script.clear();
	rpc::RpcRequest rpcRequest;
	rpcRequest.set_request_type(rpc::RpcRequest::RPCT_GET_BACKEND_INFO);
	rd->params = rpcRequest.SerializeAsString();
	return true;
}

bool GetBackendInfo::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;
	try
	{
		rpc::GetBackendInfoResult result;
		if (!util::protobuf::parseBigMessage(result, rd->response->rpc_result()))
		{
			msg("%s: util::protobuf::parseBigMessage() failed\n", __FUNCTION__);
			return false;
		}

		bitness = result.bitness();
		if (bitness != 32 && bitness != 64)
		{
			msg("%s: invalid \"bitness\" field value\n", __FUNCTION__);
			return false;
		}
		dbg_name = result.dbg_name();
		dbg_ver = result.dbg_ver();
		labeless_ver = result.labeless_ver();

		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse GetBackendInfo response\n", __FUNCTION__);
	}
	return false;
}

bool AutoCompleteCode::serialize(QPointer<RpcData> rd) const
{
	rd->script.clear();
	rpc::RpcRequest rpcRequest;
	rpcRequest.set_request_type(rpc::RpcRequest::RPCT_AUTO_COMPLETE_CODE);

	rpc::AutoCompleteCodeRequest* const request = rpcRequest.mutable_auto_complete_code_req();
	request->set_source(source);
	request->set_zline(zline);
	request->set_zcol(zcol);
	request->set_call_sig_only(callSigsOnly);

	rd->script.clear();
	rd->params = rpcRequest.SerializeAsString();
	return true;
}

bool AutoCompleteCode::parseResponse(QPointer<RpcData> rd)
{
	if (!ICommand::parseResponse(rd))
		return false;
	try
	{
		rpc::AutoCompleteCodeResult result;
		if (!util::protobuf::parseBigMessage(result, rd->response->rpc_result()))
		{
			msg("%s: util::protobuf::parseBigMessage() failed\n", __FUNCTION__);
			return false;
		}

		const auto& completions = result.completions();
		for (auto it = completions.begin(), end = completions.end(); it != end; ++it)
		{
			jresult->completions.append(QString::fromStdString(*it));
		}

		const auto& callSigs = result.call_sigs();
		for (auto it = callSigs.begin(), end = callSigs.end(); it != end; ++it)
		{
			jedi::SignatureMatch sm;
			if (it->has_cs_type())
				sm.type = QString::fromStdString(it->cs_type());
			sm.name = QString::fromStdString(it->name());
			sm.argIndex = it->index();
			if (it->has_raw_doc())
				sm.rawDoc = QString::fromStdString(it->raw_doc());

			const auto& params = it->params();
			for (auto paramIt = params.begin(), paramEnd = params.end(); paramIt != paramEnd; ++paramIt)
			{
				jedi::FuncArg arg;
				arg.name = QString::fromStdString(paramIt->name());
				if (paramIt->has_description())
					arg.description = QString::fromStdString(paramIt->description());
				sm.args.append(arg);
			}
			jresult->sigMatches.append(sm);
		}

		return true;
	}
	catch (std::runtime_error e)
	{
		msg("%s: Runtime error: %s\n", __FUNCTION__, e.what());
	}
	catch (...)
	{
		msg("%s: Unable to parse AutoCompleteCode response\n", __FUNCTION__);
	}
	return false;
}
