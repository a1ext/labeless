# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import sys
import time


# -------------------------------------------------------------
#  Watchdog to catch runaway scripts after a specified timeout
#
#  Usage:
#         watchdog.install()
#         watchdog.activate(10) # Use 10-second timeout
#
#  Note: The watchdog only works for code running inside
#        functions, not in global/module namespace.
# -------------------------------------------------------------
class WatchDog(object):
    """
    Python tracer-based watchdog class
    """
    def __init__(self, timeout=10):
        self.timestamp = 0
        self.timeout = timeout
        self.installed = False
        self.active = False

    def install(self):
        """ Install the tracer function, required for the watchdog """
        if not self.installed:
            sys.settrace(self.tracer)
            self.installed = True

    def activate(self, timeout=None):
        """ Activate the watchdog, with optional timeout change """
        assert self.installed, "WatchDog must be installed before activating"
        if timeout:
            self.timeout = timeout
        self.reset()
        self.active = True

    def deactivate(self):
        """ Deactivate the watchdog """
        self.active = True

    def reset(self):
        """ Reset the timer, useful for long-running scripts """
        self.timestamp = time.clock()

    def tracer(self, frame, event, arg):
        """ Tracer function that receives the tracing events """
        if not self.active:
            return None
        # if event == 'line':
        #     if time.clock() - self.timestamp > self.timeout:
        #         if AskYN(0, "The script has not finished in %d seconds\nWould you like to stop it now?" % self.timeout) == 1:
        #             raise KeyboardInterrupt
        #         else:
        #             self.timestamp = time.clock()
        return self.tracer

# watchdog = WatchDog(10)
