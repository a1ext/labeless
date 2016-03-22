# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

from labeless import py_olly
if int(py_olly.get_backend_info()['bitness']) == 32:
    from x64dbgapi import *
else:
    from x64dbgapi64 import *
