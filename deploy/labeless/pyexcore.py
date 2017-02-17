# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import sys
import py_olly
import traceback
import labeless
from labeless import api

try:
    import rpc_pb2 as rpc
except ImportError as e:
    py_olly.olly_log('protobuf is not installed in python')
    raise e


class binary_result(object):
    """
        Methods marked with this decorator should return objects from rpc.*
    """
    def __init__(self, f):
        self._f = f

    def __call__(self, *args, **kwargs):
        job_id, v = self._f(*args, **kwargs)
        if v:
            serialized = v.SerializeToString()
            v1 = v.__class__()
            v1.ParseFromString(serialized)
            print 'verify: %r, len: %u, name: %s' % (v1 == v, len(serialized), v.__class__.__name__)
            py_olly.set_binary_result(job_id, serialized)


class PyExCore(object):
    ROUTES = {
        rpc.RpcRequest.RPCT_MAKE_NAMES:             ('make_names_req', '_rpc_make_names'),
        rpc.RpcRequest.RPCT_MAKE_COMMENTS:          ('make_comments_req', '_rpc_make_comments'),
        rpc.RpcRequest.RPCT_GET_MEMORY_MAP:         (None, '_rpc_get_memory_map'),
        rpc.RpcRequest.RPCT_READ_MEMORY_REGIONS:    ('read_memory_regions_req', '_rpc_read_memory_regions'),
        rpc.RpcRequest.RPCT_ANALYZE_EXTERNAL_REFS:  ('analyze_external_refs_req', '_rpc_analyze_external_refs'),
        rpc.RpcRequest.RPCT_CHECK_PE_HEADERS:       ('check_pe_headers_req', '_rpc_check_pe_headers'),
        rpc.RpcRequest.RPCT_GET_BACKEND_INFO:       (None, '_rpc_get_backend_info'),
        rpc.RpcRequest.RPCT_AUTO_COMPLETE_CODE:     ('auto_complete_code_req', '_rpc_auto_complete_code')
    }

    @classmethod
    def execute(cls, job_id):
        try:
            raw_command = py_olly.get_params(job_id)
            if raw_command is None:
                print >> sys.stderr, 'Invalid rpc params'
                return
            r = rpc.RpcRequest()
            r.ParseFromString(raw_command)

            if not labeless.is_paused():
                print >> sys.stderr, 'Warn! calling RPC on non-paused debuggee may cause an unpredictable results'

            if r.request_type not in cls.ROUTES:
                print >> sys.stderr, 'Invalid request type: ', r.request_type
                return  # TODO: return error

            call_meta = cls.ROUTES[r.request_type]
            req = None if call_meta[0] is None else call_meta[0]
            if not req is None and hasattr(r, req):
                req = getattr(r, req, None)

            getattr(cls, call_meta[1])(req, job_id)
        except Exception as exc:
            print >> sys.stderr, 'Exception: %r\r\n%s' % (exc, traceback.format_exc().replace('\n', '\r\n'))
            py_olly.set_error(job_id, '%r' % exc)

    @classmethod
    @binary_result
    def _rpc_make_names(cls, req, job_id):
        return job_id, labeless.make_names(req.names, req.base, req.remote_base)

    @classmethod
    @binary_result
    def _rpc_make_comments(cls, req, job_id):
        return job_id, labeless.make_comments(req.names, req.base, req.remote_base)

    @classmethod
    @binary_result
    def _rpc_get_memory_map(cls, req, job_id):
        return job_id, labeless.get_memory_map()

    @classmethod
    @binary_result
    def _rpc_read_memory_regions(cls, req, job_id):
        return job_id, labeless.read_memory_regions(req.regions)

    @classmethod
    @binary_result
    def _rpc_analyze_external_refs(cls, req, job_id):
        #py_olly.olly_log('_rpc_analyze_external_refs: %08X-%08X inc by %08X' % (req.ea_from, req.ea_to, req.increment))
        return job_id, labeless.analyze_external_refs(req.ea_from, req.ea_to, long(req.increment), req.analysing_base, req.analysing_size)

    @classmethod
    @binary_result
    def _rpc_check_pe_headers(cls, req, job_id):
        return job_id, labeless.check_pe_headers(req.base, req.size)

    @classmethod
    @binary_result
    def _rpc_get_backend_info(cls, req, job_id):
        return job_id, labeless.get_backend_info()

    @classmethod
    @binary_result
    def _rpc_auto_complete_code(cls, req, job_id):
        return job_id, labeless.auto_complete_code(req.source, req.zline, req.zcol, req.call_sig_only)
