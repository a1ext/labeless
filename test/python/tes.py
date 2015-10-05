from logs import make_logger
import sys
import threading
import rpc_pb2 as rpc
import distorm3
import multiprocessing
import traceback
import time

logger = make_logger()

modules_exports = {}


def scan_for_ref_api_calls(ea_from, ea_to, increment, rv, base, mem=None):
    # import inspect
    if ea_from > ea_to:
        print >> sys.stderr, 'Invalid arguments passed'
        return None
    logger.info('scan_for_ref_api_calls(ea_from=0x%08X, ea_to=0x%08X, increment=0x%08X, base=0x%08X)\ngetting modules meta' % (ea_from, ea_to, increment, base))

    #modules_meta = get_modules_meta()
    global modules_exports

    logger.info('scan_for_ref_api_calls() modules_meta got')

    #main_module = oa.Findmodule(base)
    #comment = bytearray(oa.TEXTLEN)

    def isPointsToExternalDll(addr):
        if addr in modules_exports:
            return modules_exports[addr]
        return False
        # name = bytearray(2048)
        # n = oa.Decodeaddress(addr, 0, oa.ADC_SYMBOL | oa.ADC_ENTRY,  # oa.ADC_VALID | oa.ADC_JUMP
        #                      name, 2048, comment)
        # if not n or not name:
        #     return False
        # name = name.split('.')
        # if len(name) != 2:
        #     return False
        # name = str(name[1].replace('\x00', ''))
        # m = oa.Findmodule(addr)
        # if not m or not m.path or m.path == main_module.path:
        #     return False
        #
        # module_name = path.basename(m.path)
        # s = module_name.split('.')
        # if len(s) > 1:
        #     s.pop(len(s) - 1)
        # module_name = '.'.join(s)
        # return [module_name.upper(), name]

    # if mem is None:
    #     mem = safe_read_chunked_memory_region_as_one(ea_from, ea_to - ea_from)
    #     if not mem:
    #         print >> sys.stderr, 'Unable to read specified memory (0x%08X - 0x%08X)' % (ea_from, ea_to)
    #         return None
    #     mem = buffer(mem[1])

    #disasms = {}

    rv_lock = threading.RLock()

    def process_chunk(start, end):
        logger.info('processing chunk: start: 0x%08X, end: 0x%08X' % (start, end))
        for ea in xrange(start, end, increment):
            try:
                l = end - ea
                offs = ea - base
                cmd = mem[offs: offs + min(16, l)]
                # dis = oa.t_disasm()
                # n = oa.Disasm(cmd, len(cmd), ea, None, dis, oa.DISASM_CODE, 0)
                # if n <= 0 or dis.error:
                #     continue
                instr = distorm3.Decompose(ea, cmd)
                if not instr or not instr[0].valid or not instr[0].operands:
                    continue
                instr = instr[0]


                #disasms[ea] = {'l': n, 'd': dis.result}

                #bf = bytearray(127)
                #nback = oa.Disassembleback(bf, ea_from, ea_to - ea_from, ea, 1, 1)
                #if nback:
                #    if nback in disasms and disasms[nback]['l'] + nback > ea:
                #        print >> sys.stderr, 'EA: %08X skipping overlapped insn, dis: %s' % (ea, disasms[nback]['d'])
                #        continue

                # o = {
                #     'ea': ea,
                #     'len': n,
                #     'dis': dis.result,
                #     'ref': {}
                # }
                # t = None
                operand = None
                for op in instr.operands:
                    if op.type in (distorm3.OPERAND_ABSOLUTE_ADDRESS, distorm3.OPERAND_IMMEDIATE, distorm3.OPERAND_MEMORY):
                        operand = op
                        break
                if not operand:
                    continue

                if operand.type == distorm3.OPERAND_IMMEDIATE and operand.size == 32 and operand.value:
                    p = operand.value
                    v = isPointsToExternalDll(p)
                    if v:
                        with rv_lock:
                            ref = rv.refs.add()
                            ref.ref_type = rpc.AnalyzeExternalRefsResult.RefData.REFT_IMMCONST
                            ref.module, ref.proc = v.split('.')
                            ref.v = p
                            ref.ea = ea
                            ref.len = instr.size
                            ref.dis = instr._toText()
                        print 'dis.immconst points to %s at %08X' % (v, ea)
                    continue
                if operand.type == distorm3.OPERAND_MEMORY and operand.dispSize == 32 and operand.disp:
                    p = operand.disp
                    v = isPointsToExternalDll(p)
                    if v:
                        with rv_lock:
                            ref = rv.refs.add()
                            ref.ref_type = rpc.AnalyzeExternalRefsResult.RefData.REFT_JMPCONST
                            ref.module, ref.proc = v.split('.')
                            ref.v = p
                            ref.ea = ea
                            ref.len = instr.size
                            ref.dis = instr._toText()
                        print 'dis.jmpconst points to %s at %08X' % (v, ea)
                    continue

                if operand.type == distorm3.OPERAND_ABSOLUTE_ADDRESS and \
                        (operand.size == 32 or operand.dispSize == 32) and \
                        operand.disp:
                    p = operand.disp
                    v = isPointsToExternalDll(p)
                    if v:
                        with rv_lock:
                            ref = rv.refs.add()
                            ref.ref_type = rpc.AnalyzeExternalRefsResult.RefData.REFT_ADDRCONST
                            ref.module, ref.proc = v.split('.')
                            ref.v = p
                            ref.ea = ea
                            ref.len = instr.size
                            ref.dis = instr._toText()
                        print 'dis.adrconst points to %s at %08X' % (v, ea)
                    continue

                #for k, v in inspect.getmembers(dis):
                #    if '_' not in k:
                #        print "%r: %r" % (k, v)
            except Exception as exc:
                print >> sys.stderr, 'Exception: %r\r\n%s' % (exc, traceback.format_exc().replace('\n', '\r\n'))
        logger.info('scan_for_ref_api_calls() worker: "%s" done' % threading.current_thread().name)

    if ea_to - ea_from < 1024 * 1024 or True:  # use thread pool
        logger.info('scan_for_ref_api_calls() using direct processing')
        process_chunk(ea_from, ea_to)
    else:
        pool = list()
        preferred_cpu_count = max(1, multiprocessing.cpu_count())
        chunks_size = (ea_to - ea_from) / preferred_cpu_count
        logger('scan_for_ref_api_calls() using threaded processing:\n' +
               'threads: %u, chunk_size: 0x%08X' % (preferred_cpu_count, chunks_size))
        for i in xrange(preferred_cpu_count):
            chunk_begin = ea_from + (chunks_size * i)
            chunk_end = min(chunk_begin + chunks_size + 16, ea_to)
            pool.append((threading.Thread(target=process_chunk, args=(chunk_begin, chunk_end), name=('worker %u' % i)), False))
            logger.info('scan_for_ref_api_calls() added worker %u, chunk_begin: 0x%08X, chunk_end: 0x%08X' % (i, chunk_begin, chunk_end))
        for th in pool:
            th[0].start()

        while all([not x[0].is_alive() for x in pool]):
            logger.info('scan_for_ref_api_calls() alive threads: %s' % ', '.join(map(lambda x: x.name, threading.enumerate())))
            time.sleep(1)

if __name__ == '__main__':
    with open('d:\\pe_test.bin', 'rb') as f:
        raw = f.read()
    import time
    rv = rpc.AnalyzeExternalRefsResult()
    start_time = time.time()
    scan_for_ref_api_calls(0x6ef41000, 0x6F0AC000, 1, rv, 0x6ef40000, raw)
    end_time = time.time()
    print 'execution time: %u sec' % (end_time - start_time)