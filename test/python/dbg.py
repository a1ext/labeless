# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons CC BY-NC-ND 3.0
# http://creativecommons.org/licenses/by-nc-nd/3.0/

import sys
import traceback
import ollyutils as ou
import ollyapi as oa
import ctypes as C
from ctypes.wintypes import MSG
from time import time


class Dbg(object):
    pausedex_handlers = list()

    @staticmethod
    def force_process_messages(delay_sec):
        PM_REMOVE = 0x0001
        PM_NOREMOVE = 0x0000
        WM_QUIT = 0x0012
        user32 = C.windll.user32

        end = time() + min(1, abs(delay_sec))
        while time() < end:
            msg = MSG()
            if user32.PeekMessageA(C.byref(msg), 0, 0, 0, PM_NOREMOVE):
                if msg.message == WM_QUIT:
                    return
                is_uni = user32.IsWindowUnicode(msg.hWnd)

                PeekMessage = user32.PeekMessageW if is_uni else user32.PeekMessageA
                DispatchMessage = user32.DispatchMessageW if is_uni else user32.DispatchMessageA

                exists = PeekMessage(C.byref(msg), 0, 0, 0, PM_REMOVE)
                if not exists:
                    continue
                user32.TranslateMessage(C.byref(msg))
                DispatchMessage(C.byref(msg))

    @staticmethod
    def add_handler(handler):
        Dbg.pausedex_handlers.append(handler)

    @staticmethod
    def remove_handler(handler):
        if handler in Dbg.pausedex_handlers:
            Dbg.pausedex_handlers.pop(Dbg.pausedex_handlers.index(handler))

    @staticmethod
    def on_pausedex(reasonex, dummy, reg, debugevent):
        for handler in Dbg.pausedex_handlers:
            try:
                handler.on_pausedex(reasonex, dummy, reg, debugevent)
            except Exception as exc:
                print >> sys.stderr, 'Exception occurred: %r\n%s' % (exc, traceback.format_exc())

    @staticmethod
    def on_reset():
        pass

    @staticmethod
    def on_close():
        pass

    @staticmethod
    def on_destroy():
        while Dbg.pausedex_handlers:
            handler = Dbg.pausedex_handlers.pop()
            handler.on_destroy()
            del handler

    @staticmethod
    def step_in():
        oa.Go(0, 0, oa.STEP_IN, 0, 0)
        # Dbg.force_process_messages(1)

    @staticmethod
    def step_over():
        oa.Go(0, 0, oa.STEP_OVER, 0, 0)
        # Dbg.force_process_messages(1)

    @staticmethod
    def run():
        oa.Go(0, 0, oa.STEP_RUN, 0, 0)
        # Dbg.force_process_messages(5)

    @staticmethod
    def run_till_addr(ea):
        oa.Go(0, ea, oa.STEP_RUN, 0, 0)
        # Dbg.force_process_messages(5)


class GenericHandler(object):
    def on_pausedex(self, reasonex, dummy, reg, debugevent):
        pass

    def on_destroy(self):
        pass


if __name__ == '__main__':
    class UPXUnpacker(GenericHandler):
        def __init__(self):
            super(UPXUnpacker, self).__init__()

        def on_pausedex(self, reasonex, dummy, reg, debugevent):
            pass

        def get_ea(self):
            th = oa.Findthread(oa.Getcputhreadid())
            if th is None:
                return
            return th.reg.ip

        def run(self):
            ea = self.get_ea()
            first_instr, n = ou.disasm(ea)
            if first_instr is None:
                print >> sys.stderr, "ERROR: current IP doesn't looks like start of UPX packed executable"
                return
            Dbg.step_over()
            ea = self.get_ea()

            oa.Sethardwarebreakpoint(ea, 1, oa.HB_CODE)
            Dbg.run()
            oa.Deletehardwarebreakbyaddr(ea)

            ea = self.get_ea()
            while True:
                instr, size = ou.disasm(ea)
                if instr is None:
                    break

                if instr.result.replace('\0', '').lower().startswith('jmp'):
                    Dbg.run_till_addr(ea)
                    Dbg.step_in()
                    print 'Unpacking is done, the OEP is 0x%08x' % self.get_ea()
                    break
                ea += size

    upxUnpacker = UPXUnpacker()
    Dbg.add_handler(upxUnpacker)

    upxUnpacker.run()
