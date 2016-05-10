#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#    memory.py - High level API to manipulate memory.
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
import threads
import utils

from struct import unpack as u
from binascii import unhexlify

# Wrappers


def FlushMemoryCache():
    """
    Flush the intern memory cache of OllyDBG2
    """
    api.Flushmemorycache()


def WriteMemory(buff, addr=None, mode=0):
    """
    Write directly in the memory of the process
    """

    # XXX: check if memory exists
    if addr is None:
        addr = threads.GetEip()
        
    n = api.Writememory(
        buff,
        addr,
        len(buff),
        mode
    )

    # flush the cache after writing ; not sure it's good/required to do that though.
    FlushMemoryCache()

    return n


def ReadMemory(size, addr=None, mode=0):
    """
    Read the memory of the process at a specific address
    """
    # XXX: test if the address exists
    if addr is None:
        addr = threads.GetEip()

    b = bytearray(size)
    n = api.Readmemory(
        b,
        addr,
        size,
        mode
    )

    # XXX: Hmm, don't care about n right ?
    return str(b)


def Expression_(result, expression, data, base, size, threadid, a, b, mode):
    """
    Let OllyDbg evaluate an expression for you:
        * get an exported function address easily thanks to the notation module.function_name
    """
    r = api.Expression(
        result,
        expression,
        data,
        base,
        size,
        threadid,
        a,
        b,
        mode
    )

    if result.value == 'Unrecognized identifier':
        return None

    return r


def FindMemory(addr):
    """
    Find a structure t_memory describing the memory addr points to
    """
    return api.Findmemory(addr)


# Abstraction

def ResolveApiAddress(module, function):
    """
    Get the address of a specific API exported by a specific module thanks to their names

    Note:
        - you can use '<ModuleEntryPoint>' as a function name to resolve the entry point of a specific module
    """
    r = api.t_result()
    ret = Expression_(
        r,
        '%s.%s' % (module, function),
        '',
        0,
        0,
        threads.GetCpuThreadId(),
        0,
        0,
        0
    )

    if r.datatype == api.EXPR_DWORD:
        return r.u.u

    return None


def ReadDwordMemory(address=None):
    """
    Read a dword in memory
    """
    if address is None:
        address = threads.GetEip()

    data = ReadMemory(4, address)
    return u('<I', data)[0]


def IsMemoryExists(address):
    """
    Is the memory page exists in the process ?
    """
    return FindMemory(address) is not None


def PatchCodeWithHex(s, address=None):
    """
    Patch the code at address with unhexlify(s)
    """
    # XXX: test if the memory exists
    if address is None:
        address = threads.GetEip()

    bin = ''
    try:
        bin = unhexlify(s)
    except:
        raise Exception('You must supply a string composed exclusively of hex symbols')

    # patch the code
    WriteMemory(address, bin)


def PatchCode(s, address=None):
    """
    Assemble s and patch address
    """

    # XXX: test if the memory exists
    if address is None:
        address = threads.GetEip()

    bin = ''
    try:
        bin, s = utils.Assemble__(s)
    except Exception, e:
        raise e

    # patch the code
    WriteMemory(bin, address)
