# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

import os
import sys
import os
import time
import traceback

_path = os.path.dirname(os.path.realpath(__file__))
if _path not in sys.path:
    sys.path.append(_path)
del _path

import py_olly

class MyStdOut(object):
    def __init__(self, func):
        super(MyStdOut, self).__init__()
        self._func = func

    """
    Dummy file-like class that receives stout and stderr
    """
    def write(self, text):
        # OllyDbg can't handle newlines so strip them out
        fixed = text.replace('\n', '')
        if fixed:
            #_ollyapi.Addtolist(0, 0, fixed)
            self._func(fixed)

    def flush(self):
        pass

    def isatty(self):
        return False


# Redirect stderr and stdout to the OllyDbg log window
sys.stdout = MyStdOut(py_olly.std_out_handler)
sys.stderr = MyStdOut(py_olly.std_err_handler)

# Assign a default sys.argv
sys.argv = [""]

# Have to make sure Python finds our modules

# import rdebug
from ollyapi2 import *
from ollyutils import *

#-------------------------------------------------------------
# Watchdog to catch runaway scripts after a specified timeout
#
# Usage:
#        watchdog.install()
#        watchdog.activate(10) # Use 10-second timeout
#
# Note: The watchdog only works for code running inside
#       functions, not in global/module namespace.
#-------------------------------------------------------------
class WatchDog():
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
        #if event == 'line':
        #    if time.clock() - self.timestamp > self.timeout:
        #        if AskYN(0, "The script has not finished in %d seconds\nWould you like to stop it now?" % self.timeout) == 1:
        #            raise KeyboardInterrupt
        #        else:
        #            self.timestamp = time.clock()
        return self.tracer

#watchdog = WatchDog(10)
