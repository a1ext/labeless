# -*- coding: utf-8 -*-
__author__ = 'AlexWMF'

import socket
import json
import hashlib


def send_and_get_response(message):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('127.0.0.1', 3852))
    s.send(json.dumps(message))
    r = s.recv(1024)
    print 'received: %s' % r
    s.close()
    return r

def prepare_script(script):
    md5 = hashlib.md5()
    md5.update(script)
    return {'script': script, 'len': len(script), 'md5': md5.hexdigest()}

def add_label(addr, text):
    script = """import ollyapi
ollyapi.Insertname(0x%08X, ollyapi.NM_LABEL, \"%s\")
ollyapi.Redrawdisassembler()
""" % (addr, text.replace("\"", "\\\""))
    script = prepare_script(script)
    print send_and_get_response(script)


add_label(0x1E035BDA, "odin-odin-odin")
