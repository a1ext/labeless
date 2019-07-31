# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import datetime
import sys
import json

# import rdebug
import py_olly
from labeless.logs import MyStdOut


# Redirect stderr and stdout to the backend's log window
if not isinstance(sys.stdout, MyStdOut):
    sys.stdout = MyStdOut(py_olly.std_out_handler, sys.stdout)
if not isinstance(sys.stderr, MyStdOut):
    sys.stderr = MyStdOut(py_olly.std_err_handler, sys.stderr)

# Assign a default sys.argv
sys.argv = [""]

# Have to make sure Python finds our modules

backend_name = py_olly.get_backend_info()['name']
from backend import get_backend
LB = get_backend(backend_name)
all_names = getattr(LB, '__all__') if hasattr(LB, '__all__') else (key for key in LB.__dict__ if not key.startswith('_'))

my_ns = globals()

for name in all_names:
    my_ns[name] = getattr(LB, name)

del backend_name
del all_names
del my_ns
# if backend_name == 'ollydbg11':
#     from backend.ollydbg11 import *
# elif backend_name == 'ollydbg20':
#     from backend.ollydbg20 import *
# elif backend_name == 'x64dbg':
#     from backend.x64dbg import *
# else:
#     msg = 'labeless.__init__: invalid debug backend got: %s' % backend_name
#     py_olly.olly_log(msg)
#     raise Exception(msg)
# FIXME


class _DateTimeJsonSerializer(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, (datetime.datetime, datetime.date)):
            return obj.isoformat()

        return super(_DateTimeJsonSerializer, self).default(obj)


def serialize_result(v):
    """This method calls after each python execution"""
    # override this method or cls if you want to add custom types to be serialized
    return json.dumps(v, cls=_DateTimeJsonSerializer)

