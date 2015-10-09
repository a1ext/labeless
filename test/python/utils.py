# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

from threading import RLock


class Singleton(type):
    __instances = {}
    __instances_lock = RLock()

    def __call__(cls, *args, **kwargs):
        with cls.__instances_lock:
            if cls not in cls.__instances:
                cls.__instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls.__instances[cls]
