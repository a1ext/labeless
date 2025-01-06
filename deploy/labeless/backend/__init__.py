# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import sys
import traceback


def get_backend(name):
    try:
        module = __import__(name, globals(), locals(), ["dummy"], 0)

        # TODO

        return module
    except:
        print(traceback.format_exc(), file=sys.stderr)
        raise


# __all__ = ['get_backend']
