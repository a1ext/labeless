#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#    breakpoints.py - A python high level API to play with breakpoints in OllyDBG2
#    Copyright (C) 2012 Axel "0vercl0k" Souchet - http://www.twitter.com/0vercl0k
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import ollyapi2 as api


def SetInt3Breakpoint(address, type_bp=0, fnindex=0, limit=0, count=0, actions=0, condition='', expression='', exprtype=''):
    """
    Python wrapper for the Setint3breakpoint function
    """
    return api.Setint3breakpoint(
        address,
        type_bp,
        fnindex,
        limit,
        count,
        actions,
        condition,
        expression,
        exprtype
    )


def SetHardBreakpoint(address, index=0, size=0, type_=0, fnindex=0, limit=0, count=0, actions=0, condition='', expression='', exprtype=''):
    """
    Set a hardware breakpoint
    """
    return api.Sethardbreakpoint(
        index,
        size,
        type_,
        fnindex,
        address,
        limit,
        count,
        actions,
        condition,
        expression,
        exprtype
    )


def RemoveInt3Breakpoint(address, type_bp):
    """
    Remove software breakpoint
    """
    return api.Removeint3breakpoint(
        address,
        type_bp
    )


def RemoveHardbreapoint(slot):
    """
    Remove hardware breakpoint
    """
    return api.Removehardbreakpoint(slot)


def FindFreeHardbreakSlot(type_):
    """
    Find a free slot to put your hardware breakpoint
    """
    return api.Findfreehardbreakslot(type_)


def SetMemoryBreakpoint(address, size=1, type_=0, limit=0, count=0, condition='', expression='', exprtype=''):
    """
    Set a memory breakpoint
    """
    return api.Setmembreakpoint(
        address,
        size,
        type_,
        limit,
        count,
        condition,
        expression,
        exprtype
    )


def RemoveMemoryBreakpoint(addr):
    """
    Remove a memory breakpoint
    """
    return api.Removemembreakpoint(addr)


# WE NEED ABSTRACTION MAN

class Breakpoint(object):
    """
    """
    def __init__(self, address, type_bp, condition=None):
        self.address = address
        self.state = 'Disabled'
        self.type = type_bp
        self.is_conditional_bp = condition is not None
        self.condition = '' if condition is None else condition

    def get_address(self):
        """Get the address of the breakpoint"""
        return self.address

    def get_state(self):
        """
        Get the state of your breakpoint
        """
        return self.state

    def is_enabled(self):
        """
        Is the breakpoint enabled ?
        """
        return self.state == 'Enabled'

    def is_disabled(self):
        """
        Is the breakpoint disabled ?
        """
        return self.state == 'Disabled'

    def disable(self):
        """
        Disable the breakpoint
        """
        pass

    def remove(self):
        """
        Remove the breakpoint
        """
        pass

    def enable(self):
        """
        Enable the breakpoint
        """
        pass

    @staticmethod
    def flags_to_bp_type(flags, is_conditional_bp=False):
        """
        Translate the 'rwx' in stuff that olly understands
        """
        # remove duplicate letter
        flags = ''.join(set(flags))

        # now ensure the maximum you can do is 'rwx'
        assert(len(flags) < 4)

        # we want only 'r', 'w' & 'x' in the flags
        assert(filter(lambda x: x in ['r', 'w', 'x'], flags) != [])

        # converting the flags into valid breakpoint type
        type_dword = api.BP_MANUAL | api.BP_BREAK
        if 'r' in flags:
            type_dword |= api.BP_READ

        if 'w' in flags:
            type_dword |= api.BP_WRITE

        if 'x' in flags:
            type_dword |= api.BP_EXEC

        if is_conditional_bp != '':
            type_dword |= api.BP_COND

        return type_dword


class SoftwareBreakpoint(Breakpoint):
    """
    A class to manipulate, play with software breakpoint

    TODO:
        - disable
        - .continue(x) -> let the breakpoint be hit x times
    """
    def __init__(self, address, condition=None):
        # if this is a classic breakpoint we need to set different flag
        t = api.BP_MANUAL | ((api.BP_COND | api.BP_CONDBREAK) if condition is not None else api.BP_BREAK)

        # init internal state of the breakpoint
        super(SoftwareBreakpoint, self).__init__(address, t, condition)

        # enable directly the software breakpoint
        self.enable()

    def enable(self):
        if self.state != 'Enabled':

            r = SetInt3Breakpoint(
                self.address,
                self.type,
                condition = self.condition
            )

            self.state = 'Enabled'
            return r

    def remove(self):
        # we remove the breakpoint only if it is enabled
        if self.state == 'Enabled':
            RemoveInt3Breakpoint(self.address, self.type)
            self.state = 'Disabled'


class HardwareBreakpoint(Breakpoint):
    """
    A class to manipulate, play with hardware breakpoint

    Note:
        - do not use .goto() if you have a read/write bp, because you don't know where
        the breakpoint is going to be hit
    """
    def __init__(self, address, flags='x', size=1, condition=None, slot=None):
        # init internal state of the breakpoint
        super(HardwareBreakpoint, self).__init__(address, flags, condition)

        assert(size in [1, 2, 4])

        # DR7 allows only 3 types of HWBP:
        #  -> on execution (00b)
        #  -> data write (01b)
        #  -> data read or write (11b)
        assert(flags in ['rw', 'w', 'wr', 'x'])

        # ensure the size is 1byte if this is an execution breakpoint
        if flags == 'x' :
            size = 1

        self.size = size

        # keep in memory the flags view like 'rw' breakpoint, but translate it into something
        # ollydbg understands
        self.internal_type = Breakpoint.flags_to_bp_type(self.type, self.is_conditional_bp)

        self.slot = slot if slot is not None else FindFreeHardbreakSlot(self.internal_type)
        if self.slot == -1:
            raise Exception('You have used all the available slot')
        
        self.enable()

    def enable(self):
        if self.state != 'Enabled':
            r = SetHardBreakpoint(
                self.address,
                self.slot,
                self.size,
                self.internal_type,
                condition = self.condition
            )

            self.state = 'Enabled'
            return r

    def remove(self):
        if self.state == 'Enabled':
            RemoveHardbreapoint(self.slot)
            self.state = 'Disabled'


class MemoryBreakpoint(Breakpoint):
    """
    A class to manipulate memory breakpoints
    """
    def __init__(self, address, flags='x', size=1, condition=''):
        super(MemoryBreakpoint, self).__init__(address, flags, condition)
        self.internal_type = Breakpoint.flags_to_bp_type(self.type, self.is_conditional_bp)
        self.size = size

        self.enable()

    def enable(self):
        if self.state != 'Enabled':
            r = SetMemoryBreakpoint(
                self.address,
                self.size,
                self.internal_type,
                condition=''
            )

            self.state = 'Enabled'
            return r

    def remove(self):
        if self.state == 'Enabled':
            RemoveMemoryBreakpoint(self.address)
            self.state = 'Disabled'
