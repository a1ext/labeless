# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import sys
egg_location = r'c:\windows\Python27\Lib\site-packages\pycharm-debug.egg'
if egg_location not in sys.path:
    sys.path.append(egg_location)

import pydevd_pycharm
pydevd_pycharm.settrace('localhost', port=11111) #, stdoutToServer=False, stderrToServer=False, suspend=True) # , stdoutToServer=True, stderrToServer=True
