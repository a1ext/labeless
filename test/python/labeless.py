# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons CC BY-NC-ND 3.0
# http://creativecommons.org/licenses/by-nc-nd/3.0/

__author__ = 'a1ex_t'

import ctypes as C
import errno
import json
import select
import socket
from SocketServer import BaseRequestHandler, TCPServer
import sys
import threading
import traceback
from rpc_pb2 import Execute
import ollyapi as oa
from utils import Singleton
import Queue


class _Request(object):
    def __init__(self):
        self.id = 0
        self.script = ''
        self.script_extern_obj = ''
        self.params = ''
        self.result = ''
        self.binary_result = ''
        self.error = ''
        self.is_finished = False
        self.is_background = False
        self.handler = None


class _Client(object):
    def __init__(self):
        self.sock = None
        self.peer = ''
        self.net_buff = None
        self.cmds_lock = threading.RLock()
        self.cmds = list()

        self.std_out_buff = ''
        self.std_out_buff_lock = threading.RLock()
        self.std_err_buff = ''
        self.std_err_buff_lock = threading.RLock()


class _ThreadAccessBackend(object):
    def server_log(self, message):
        raise NotImplementedError()

    def server_thread_finished(self):
        raise NotImplementedError()

    def server_port_changed(self):
        raise NotImplementedError()

    def server_command_received(self, cmd):
        raise NotImplementedError()


class _NonThreadSafePyExecKludgeWin(_ThreadAccessBackend):
    WS_CHILD = 0x40000000L
    WS_POPUP = 0x80000000L
    GWL_WNDPROC = -4
    WM_DESTROY = 2

    def __init__(self, ll, hinstance=None, parent_hwnd=0):
        self.ll = ll  # Labeless
        self.hwnd = None
        self.hinstance = hinstance
        self.parent_hwnd = parent_hwnd

        self.hlpLogMessageId = None
        # self.hlpServerThrFinishedId = None
        self.hlpCommandReceived = None
        self.hlpPortChanged = None

        self.kernel32 = C.windll.kernel32
        self.user32 = C.windll.user32
        self.IsWindow = self.user32.IsWindow
        self.CallWindowProc = self.user32.CallWindowProcW
        self.old_wndproc = None

        self.params_lock = threading.RLock()
        self.param_id = 0
        self.params = dict()

        if not self._create_window():
            print >> sys.stderr, '_create_window() failed'

    def destroy(self):
        if self.hwnd:
            self.user32.DestroyWindow(self.hwnd)
            self.hwnd = None

    def _find_parent_wnd(self):
        EnumProc = C.WINFUNCTYPE(C.c_ulong, C.c_ulong, C.POINTER(C.c_int))
        IsVisible = self.user32.IsWindowVisible
        pid = self.kernel32.GetProcessId(self.kernel32.GetCurrentProcess())
        found_parent = C.c_int(0)

        def enum_proc(hwnd, lparam):
            global found_parent
            dwPid = C.c_int()
            if not self.user32.GetWindowThreadProcessId(hwnd, C.byref(dwPid)):
                return True
            if dwPid.value == pid:
                parent = hwnd
                h = parent
                while h:
                    h = self.user32.GetParent(h)
                    if h and IsVisible(h):
                        parent = h
                if parent:
                    lparam[0] = parent
                    # bff = C.create_unicode_buffer(256)
                    # self.user32.GetWindowTextW(parent, C.byref(bff), 256)

                    # print 'parent got %08X, "%s"' % (parent, str(bff.value).replace('\0', ''))
                    return 0

            return 1
        self.user32.EnumWindows(EnumProc(enum_proc), C.byref(found_parent))
        return found_parent.value

    def _create_window(self):
        if self.hwnd:
            return

        if not self.hinstance:
            self.hinstance = self.kernel32.GetModuleHandleW(0)
        if not self.parent_hwnd:
            self.parent_hwnd = self._find_parent_wnd()
        print 'hinst: %08X, parent: %08X' % (self.hinstance, self.parent_hwnd)
        flags = self.WS_CHILD if self.parent_hwnd else self.WS_POPUP
        self.hwnd = self.user32.CreateWindowExW(0, C.c_wchar_p('STATIC'), C.c_wchar_p('Labeless'), flags, 0, 0, 100, 100,
                                                self.parent_hwnd, 0, self.hinstance, 0)
        if not self.hwnd:
            raise Exception('Unable to create helper window, le:%08X' % self.kernel32.GetLastError())

        WndProcType = C.WINFUNCTYPE(C.c_int, C.c_long, C.c_int, C.c_int, C.c_int)
        self.old_wndproc = self.user32.SetWindowLongW(self.hwnd, C.c_int(self.GWL_WNDPROC), WndProcType(self._wnd_proc))

        self.hlpLogMessageId = self.user32.RegisterWindowMessageW(C.c_wchar_p("{B221E840-FBD2-4ED3-A69E-3DDAB1F7EC36}"))
        if not self.hlpLogMessageId:
            print >> sys.stdout, "RegisterWindowMessageW(hlpLogMessageId) failed. LastError: %08X" % self.kernel32.GetLastError()
            return False
        self.hlpCommandReceived = self.user32.RegisterWindowMessageW(C.c_wchar_p("{79F0D105-76FF-40DB-9448-E9D9E5BA7938}"))
        if not self.hlpCommandReceived:
            print >> sys.stdout, "RegisterWindowMessageW(hlpCommandReceived) failed. LastError: %08X" % self.kernel32.GetLastError()
            return False
        self.hlpPortChanged = self.user32.RegisterWindowMessageW(C.c_wchar_p("{774A37C9-6398-44AD-8F07-A421B55F0435}"))
        if not self.hlpPortChanged:
            print >> sys.stdout, "RegisterWindowMessageW(hlpPortChanged) failed. LastError: %08X" % self.kernel32.GetLastError()
            return False
        return True

    def _take_param(self, param_id):
        with self.params_lock:
            if param_id in self.params:
                return self.params.pop(param_id)

    def _wnd_proc(self, hwnd, msg, wparam, lparam):
        if msg == self.hlpCommandReceived:
            cmd = self._take_param(wparam)
            if cmd:
                if not self.ll.on_command_received(cmd):
                    print >> sys.stderr, 'on_command_received() failed'
        elif msg == self.hlpPortChanged:
            self.ll.on_port_changed()
        elif msg == self.hlpLogMessageId:
            message = self._take_param(wparam)
            if message:
                oa.Addtolist(0, 1, message)
        elif msg == self.WM_DESTROY:
            self.hwnd = None
        else:
            return self.CallWindowProc(self.old_wndproc, hwnd, msg, wparam, lparam)
        return 0

    def server_port_changed(self):
        if not self.hwnd or not self.IsWindow(self.hwnd):
            return
        self.user32.PostMessage(self.hwnd, self.hlpPortChanged, 0, 0)

    def server_command_received(self, cmd):
        if not self.hwnd or not self.IsWindow(self.hwnd):
            return
        with self.params_lock:
            self.param_id += 1
            param_id = self.param_id
            self.params[self.param_id] = cmd
        self.user32.PostMessage(self.hwnd, self.hlpCommandReceived, param_id, 0)

    def server_log(self, message):
        if not self.hwnd or not self.IsWindow(self.hwnd):
            return
        with self.params_lock:
            self.param_id += 1
            param_id = self.param_id
            self.params[self.param_id] = message
        self.user32.PostMessage(self.hwnd, self.hlpLogMessageId, param_id, 0)


def _get_helper(ll):
    if sys.platform == 'win32':
        return _NonThreadSafePyExecKludgeWin(ll)
    else:
        raise Exception('Platform "%s" not supported' % sys.platform)


# class _Server(TCPServer):
#     allow_reuse_address = False
#
#     def __init__(self, ll, server_address, RequestHandlerClass, bind_and_activate=True):
#         TCPServer.__init__(self, server_address, RequestHandlerClass, bind_and_activate)
#         self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
#         self.ll = ll
#
#     def serve_forever(self, poll_interval=0.5):
#         print 'ok, binded at %s:%u' % self.socket.getsockname()
#         try:
#             while not Labeless.EXIT_EVENT.isSet():
#                 # XXX: Consider using another file descriptor or
#                 # connecting to the socket to wake this up instead of
#                 # polling. Polling reduces our responsiveness to a
#                 # shutdown request and wastes cpu at all other times.
#                 r, w, e = select.select([self], [], [], poll_interval)
#                 if self in r:
#                     self._handle_request_noblock()
#         finally:
#             print 'stopping'
#             self.shutdown()
#
#
# class _Handler(BaseRequestHandler):
#     __DEF_RECV_SIZE = 1024 * 1024
#
#     def __init__(self, request, client_address, server):
#         BaseRequestHandler.__init__(self, request, client_address, server)
#         self._ll = Labeless()
#
#     def _safe_read(self):
#         rv = None
#         try:
#             data = self.request.recv(self.__DEF_RECV_SIZE)
#             if not data:
#                 return rv
#             if rv is None:
#                 rv = data
#             else:
#                 rv += data
#         except Exception:
#             print >> sys.stderr, traceback.format_exc()
#         return rv
#
#     def _send(self, data):
#         total_sent = 0
#         while total_sent < len(data):
#             sent = self.request.sendall(data)
#             total_sent += sent
#
#     def handle(self):
#         print 'handle'
#         try:
#             self.request.setblocking(False)
#             data = self._safe_read()
#             if not data:
#                 print >> sys.stderr, 'No data read'
#                 return
#
#             print data
#             e = Execute()
#             if not e.ParseFromString(data):
#                 print >> sys.stderr, 'Unable to parse command'
#                 return
#             req = _Request()
#             req.handler = self
#             with Labeless.REQUEST_ID_LOCK:
#                 Labeless.REQUEST_ID += 1
#                 req.id = Labeless.REQUEST_ID
#             req.script = e.script()
#             req.script_extern_obj = e.script_extern_obj()
#             req.params = e.rpc_request()
#             req.is_background = e.background()
#
#             self._ll.helper.server_command_received(req)
#
#
#             # self.request.sendall('ok')
#         except Exception:
#             print >> sys.stderr, traceback.format_exc()


class Labeless(object):
    __metaclass__ = Singleton

    SERVER_THREAD = None
    SERVER_THREAD_LOCK = threading.RLock()
    EXIT_EVENT = threading.Event()
    REQUEST_ID_LOCK = threading.RLock()
    REQUEST_ID = long(0)

    POOL_INTERVAL = 0.5
    __DEF_RECV_SIZE = 1024 * 1024
    __DEF_RECV_SEND_TIMEOUT = 30 * 60 * 1000

    def __init__(self):
        self._server_host = '127.0.0.1'
        self._server_port = 0
        self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.client = _Client()
        self.helper = _get_helper(self)
        if not self.helper:
            return

    @staticmethod
    def store_port(port):
        # TODO
        pass

    def on_command_received(self, cmd):
        extern_obj = json.loads(cmd.script_extern_obj) if cmd.script_extern_obj else None
        try:
            eval(cmd.script, globals=globals().copy().update({'__extern__': extern_obj}))
            return True
        except:
            print >> sys.stderr, traceback.format_exc()

    def _server_bind_and_listen(self, host, port):
        while port < 0xFFFF:
            try:
                self._server_socket.bind((host, port))
                self._server_host, self._server_port = self._server_socket.getsockname()
                self._server_socket.listen(5)
                return True
            except:
                port += 1
        return False

    def _server_safe_read(self, sock):
        rv = None
        try:
            data = sock.recv(self.__DEF_RECV_SIZE)
            if not data:
                return rv
            if rv is None:
                rv = data
            else:
                rv += data
        except Exception:
            print >> sys.stderr, traceback.format_exc()
        return rv

    def __on_buff_received(self, data):
        print data
        e = Execute()
        if not e.ParseFromString(data):
            print >> sys.stderr, 'Unable to parse command'
            return
        req = _Request()
        req.handler = self
        with Labeless.REQUEST_ID_LOCK:
            Labeless.REQUEST_ID += 1
            req.id = Labeless.REQUEST_ID
        req.script = e.script()
        req.script_extern_obj = e.script_extern_obj()
        req.params = e.rpc_request()
        req.is_background = e.background()

        self.helper.server_command_received(req)

    def server_thread(self, host, port):
        self._server_host = host

        def _eintr_retry(func, *args):
            """restart a system call interrupted by EINTR"""
            while True:
                try:
                    return func(*args)
                except (OSError, select.error) as e:
                    if e.args[0] != errno.EINTR:
                        raise

        if not self._server_bind_and_listen(host, port):
            return False
        if port != self._server_port:
            self.helper.server_port_changed()
        print 'ok, binded at %s:%u' % (self._server_host, self._server_port)
        try:
            inputs = [self._server_socket]
            while not Labeless.EXIT_EVENT.isSet():
                r, w, e = _eintr_retry(select.select, inputs, [], [], self.POOL_INTERVAL)
                if self._server_socket in r:
                    client, cli_addr = self._server_socket.accept()
                    client.setblocking(0)
                    client.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
                    client.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, self.__DEF_RECV_SEND_TIMEOUT)
                    client.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, self.__DEF_RECV_SEND_TIMEOUT)

                    inputs.append(client)

                    if self.client.sock is not None:
                        self.client.sock.close()
                    self.client.sock = client
                    self.client.peer = '%s:%u' % (self._server_host, self._server_port)
                else:
                    if self.client.sock is not None:
                        with self.client.cmds_lock:
                            if self.client.cmds:
                                c = self.client.cmds[-1]
                                if c.finished and c.result:
                                    self.client.sock.sendall(c.result)
                                    self.client.sock.close()
                                    if self.client.sock in inputs:
                                        inputs.pop(inputs.index(self.client.sock))
                                    self.client.sock = None
                                    print 'jobId: %0u Response sent, len: 0x%08X' % (c.id, len(c.result))
                                    if c.background:
                                        c.finished = True
                                        c.result = None
                                    else:
                                        self.client.cmds.pop()
                    for s in r:
                        if s == self.client.sock:
                            data = self._server_safe_read(s)
                            if not data:
                                continue
                            self.__on_buff_received(data)
                    for ex in e:
                        print 'exceptional: %08X, closing' % (ex,)
                        if ex in inputs:
                            inputs.pop(inputs.index(ex))
                        if ex == self.client.sock:
                            self.client.sock = None
                        ex.close()

        finally:
            print 'stopping'
            if self.client.sock is not None:
                self.client.sock.close()
            self._server_socket.close()
            self._server_socket = None



def start_server(host='0.0.0.0', port=3852):
    ll = Labeless()

    def _thread_handler():
        try:
            ll.server_thread(host, port)
        except:
            pass

    with Labeless.SERVER_THREAD_LOCK:
        if Labeless.SERVER_THREAD is not None:
            return True

        Labeless.EXIT_EVENT.clear()
        Labeless.SERVER_THREAD = threading.Thread(target=_thread_handler)
        Labeless.SERVER_THREAD.daemon = True
        Labeless.SERVER_THREAD.start()
    # Labeless.SERVER_THREAD.join()


def stop_server():
    with Labeless.SERVER_THREAD_LOCK:
        if Labeless.SERVER_THREAD is None:
            return
        Labeless.EXIT_EVENT.set()
        Labeless.SERVER_THREAD.join()
        Labeless.SERVER_THREAD = None


if __name__ == '__main__':
    start_server()
