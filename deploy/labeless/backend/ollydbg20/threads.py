#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#    threads.py - High level API to play with threads related stuff.
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

# Wrappers


def ResumeAllThreads():
    api.Resumeallthreads()


def ThreadRegisters(threadid):
    """
    Get the registers (SEE/FPU/General registers/etc) for the current thread debugged
    """
    return api.Threadregisters(threadid)


def GetCpuThreadId():
    """
    Get the TID of the current thread
    """
    return api.Getcputhreadid()


# The metaprogramming trick doesn't work because the ip field isn't in the t_reg.r array :(
def SetEip(eip=0):
    """
    Modify the EIP register
    """
    p_reg = api.Threadregisters(GetCpuThreadId())
    p_reg.ip = eip


def GetEip():
    """
    Get the EIP register
    """
    p_reg = api.Threadregisters(GetCpuThreadId())
    return p_reg.ip


def GetEflags():
    """
    Get the EFLAGS register
    """
    p_reg = api.Threadregisters(GetCpuThreadId())
    return p_reg.flags


def GetProcessId():
    """
    Get the PID of the debuggee
    """
    # oddata (ulong)   processid;            // Process ID of Debuggee or 0
    return api.cvar.processid


def GetProcessHandle():
    """
    Get the handle on the debuggee (obtained via OpenProcess)
    """
    # oddata (HANDLE)  process;              // Handle of Debuggee or NULL
    return api.cvar.process

# metaprogramming magixx

def CreateRegisterSetter(reg_id, reg_name):
    """
    Create dynamically a setter function for an x86 register contained in t_reg.r
    """
    def template_func(reg_value):
        r = api.Threadregisters(GetCpuThreadId())
        regs = api.ulongArray.frompointer(r.r)
        regs[reg_id] = reg_value

    f = template_func
    # adjust correctly the name of the futur function
    f.__name__ = 'Set%s' % reg_name.capitalize()
    f.__doc__  = 'Set the %s register' % reg_name.upper()

    return f.__name__, f


def CreateRegisterGetter(reg_id, reg_name):
    """
    Create dynamically a getter function for an x86 register contained in t_reg.r
    """
    def template_func():
        r = api.Threadregisters(GetCpuThreadId())
        regs = api.ulongArray.frompointer(r.r)
        return regs[reg_id]

    f = template_func
    # adjust correctly the name of the futur function
    f.__name__ = 'Get%s' % reg_name.capitalize()
    f.__doc__  = 'Get the %s register' % reg_name.upper()

    return f.__name__, f


def CreateSegRegisterGetter(seg_id, seg_name):
    """
    Create dynamically a getter function for x86 segment selectors contained in t_reg.s
    """
    def template_func():
        r = api.Threadregisters(GetCpuThreadId())
        s = api.ulongArray.frompointer(r.s)
        return s[seg_id]

    f = template_func
    # adjust correctly the name of the futur function
    f.__name__ = 'Get%s' % seg_name.capitalize()
    f.__doc__  = 'Get the %s segment selector' % seg_name.upper()

    return f.__name__, f


def BuildSettersGetters():
    """
    Create dynamically all the getters/setters function used to retrieve/set register value in
    t_reg.r
    """
    list_reg = [
        ('eax', api.REG_EAX),
        ('ecx', api.REG_ECX),
        ('edx', api.REG_EDX),
        ('ebx', api.REG_EBX),
        ('esp', api.REG_ESP),
        ('ebp', api.REG_EBP),
        ('esi', api.REG_ESI),
        ('edi', api.REG_EDI)
    ]

    list_seg = [
        ('cs', api.SEG_CS),
        ('ss', api.SEG_SS),
        ('es', api.SEG_ES),
        ('ds', api.SEG_DS),
        ('gs', api.SEG_GS),
        ('fs', api.SEG_FS)
    ]

    for reg_name, reg_id in list_reg:
        # Build the setter
        n, f = CreateRegisterSetter(reg_id, reg_name)
        globals()[n] = f

        # Build the getter
        n, f = CreateRegisterGetter(reg_id, reg_name)
        globals()[n] = f

    for seg_name, seg_id in list_seg:
        n, f = CreateSegRegisterGetter(seg_id, seg_name)
        globals()[n] = f

# it's a bit magic, instanciation of the functions!
BuildSettersGetters()


# Abstraction


def GetCurrentThreadRegisters():
    """
    Retrieve the register for the current thread debugged
    """
    current_tid = GetCpuThreadId()
    if current_tid == 0:
        return None
    return ThreadRegisters(current_tid)

def SetRegisters(r):
    """
    Modify several CPU registers at the same time
    """
    handlers = {
        'eax': SetEax,
        'ebx': SetEbx,
        'ecx': SetEcx,
        'edx': SetEdx,
        'esi': SetEsi,
        'edi': SetEdi,
        'esp': SetEsp,
        'ebp': SetEbp,
        'eip': SetEip
    }

    for reg_name, value in r.iteritems():
        if reg_name in handlers:
            handlers[reg_name](value)

def GetCurrentTEB():
    """
    Retrieve the base of the TEB
    """
    r = GetCurrentThreadRegisters()
    base = api.ulongArray.frompointer(r.base)
    return base[api.SEG_FS]


def display_global_registers():
    """
    Display only the global registers
    """
    r = GetCurrentThreadRegisters()
    p_reg = api.ulongArray.frompointer(r.r)
    print 'EAX: %#.8x, ECX: %#.8x' % (p_reg[api.REG_EAX], p_reg[api.REG_ECX])
    print 'EDX: %#.8x, EBX: %#.8x' % (p_reg[api.REG_EDX], p_reg[api.REG_EBX])
    print 'ESP: %#.8x, EBP: %#.8x' % (p_reg[api.REG_ESP], p_reg[api.REG_EBP])
    print 'ESI: %#.8x, EDI: %#.8x' % (p_reg[api.REG_ESI], p_reg[api.REG_EDI])
    print 'EIP: %#.8x' % r.ip


def display_segment_selectors():
    """
    Display the segment selectors with their bases/limits
    """
    r = GetCurrentThreadRegisters()
    s = api.ulongArray.frompointer(r.s)
    base = api.ulongArray.frompointer(r.base)
    limit = api.ulongArray.frompointer(r.limit)
    print 'ES: %#.2x (%#.8x - %#.8x), CS: %#.2x (%#.8x - %#.8x)' % (s[api.SEG_ES], base[api.SEG_ES], (base[api.SEG_ES] + limit[api.SEG_ES]), s[api.SEG_CS], base[api.SEG_CS], (base[api.SEG_CS] + limit[api.SEG_CS]))
    print 'SS: %#.2x (%#.8x - %#.8x), DS: %#.2x (%#.8x - %#.8x)' % (s[api.SEG_SS], base[api.SEG_SS], (base[api.SEG_SS] + limit[api.SEG_SS]), s[api.SEG_DS], base[api.SEG_DS], (base[api.SEG_DS] + limit[api.SEG_DS]))
    print 'FS: %#.2x (%#.8x - %#.8x), GS: %#.2x (%#.8x - %#.8x)' % (s[api.SEG_FS], base[api.SEG_FS], (base[api.SEG_FS] + limit[api.SEG_FS]), s[api.SEG_GS], base[api.SEG_GS], (base[api.SEG_GS] + limit[api.SEG_GS]))


def display_eflags():
    """
    Display the EFLAGS
    """
    r = GetCurrentThreadRegisters()

    print 'EFLAGS:'
    print 'Carry flag           : %d' % ((r.flags & api.FLAG_C) != 0)
    print 'Parity flag          : %d' % ((r.flags & api.FLAG_P) != 0)
    print 'Auxiliary carry flag : %d' % ((r.flags & api.FLAG_A) != 0)
    print 'Zero flag            : %d' % ((r.flags & api.FLAG_Z) != 0)
    print 'Sign flag            : %d' % ((r.flags & api.FLAG_S) != 0)
    print 'Single-step trap flag: %d' % ((r.flags & api.FLAG_T) != 0)
    print 'Direction flag       : %d' % ((r.flags & api.FLAG_D) != 0)
    print 'Overflow flag        : %d' % ((r.flags & api.FLAG_O) != 0)


def display_all_registers():
    """
    Display all the CPU registers: global, segment selector, eflags, etc
    """
    # global
    display_global_registers()

    # Segment selectors
    display_segment_selectors()

    # eflags
    display_eflags()
