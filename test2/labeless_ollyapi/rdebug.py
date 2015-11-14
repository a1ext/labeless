# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import os
import sys
import site

# copy pycharm-debug.egg from 'c:\Program Files (x86)\JetBrains\PyCharm X.XX\debug-eggs' folder to
#  your Python 2 site-packages directory first
try:
    import pydevd
except ImportError:
    egg_name = 'pycharm-debug.egg'
    dirs = site.getsitepackages() + [site.getusersitepackages()]
    found = False

    for d in dirs:
        full_path = os.path.join(d, egg_name)
        if os.path.isfile(full_path):
            sys.path.append(full_path)
            found = True
            break
    if not found:
        raise Exception('Unable to find pydevd package')
    import pydevd

pydevd.settrace('localhost', port=12321, stdoutToServer=True, stderrToServer=True)
