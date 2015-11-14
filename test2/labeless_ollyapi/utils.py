#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#    utils.py - High level API for the utils functions.
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
from ollyapi2 import *
import threads
import memory
import sym

# Wrappers

def InsertNameW(addr, type_, s):
    """
    That function is used to add label and comment directly on the disassembly
    (like with the shortcut ':' or ';')
    """
    return InsertnameW(
        addr,
        type_,
        s
    )

def CheckForDebugEvent():
    """
    Hum, this method seems to be very important, one of its purpose
    is to updated the thread registers retrieved thanks to Threadregisters()
    """
    return Checkfordebugevent()

def CloseProcess(confirm = 0):
    """
    Close the process being debugged
    """
    return Closeprocess(confirm)

def SetArguments(s):
    """
    Set the cmdline passed to the debuggee, exactly the same when you do "File > Set new arguments"
    """
    assert(len(s) * 2 < ARGLEN)
    cvar.arguments = unicode(s)
    
def Disasm_(c, address = 0):
    """
    Disassemble some x86 code (only one instruction though) thanks to the OllyDbg2 engine
    """
    buff = bytearray(c)
    di = t_disasm()
    size_instr = Disasm(
        buff,
        len(buff),
        address,
        None,
        di,
        DA_TEXT | DA_OPCOMM | DA_DUMP | DA_MEMORY,
        t_reg(),
        t_predict()
    )

    return size_instr, str(di.result)

def Assemble_(s, address = 0):
    """
    Assemble some x86 stuff
    """

    # the longuest x86 instruction is 15 bytes long
    code = bytearray(15)

    # XXX: it should be enough (?)
    error_msg = bytearray(256)

    sizeof_assembled = Assemble(
        s,
        address,
        code,
        len(code),
        1,
        error_msg
    )

    # you submit invalid x86 assembly
    # XXX: doesn't it exist a proper way to do that ??
    if error_msg.startswith('U\x00n\x00k\x00n\x00o\x00w\x00n'):
        return (0, 0, str(error_msg.replace('\x00', '')))

    return str(code[:sizeof_assembled]), sizeof_assembled

def AssembleAllForms(s, ip = 0):
    """
    Actually, I only use this function to obtain a t_asmod structure, to pass it
    at CompareCommand()
    """
    maxmodel = 0x80
    asmod = t_asmmodArray(maxmodel)

    # XXX: it should be enough (?)
    error_msg = bytearray(256)

    r = Assembleallforms(
        s,
        ip,
        asmod.cast(),
        maxmodel,
        7,
        error_msg
    )

    if r == 0:
        raise Exception('Cannot assembled your instruction: %s' % str(error_msg.replace('\x00', '')))

    return asmod, r

def CompareCommand(cmd, cmdsize, cmdip, model, nmodel):
    """
    Compare command, used to search instruction accross the memory
    """
    a, b = 0, 0

    return Comparecommand(
        cmd,
        cmdsize,
        cmdip,
        model.cast(),
        nmodel,
        a,
        b,
        t_disasm()
    )

def GetAnalyserComment(addr):
    """
    Get an analyser comment with an address

    Example of comment you can retrieved:

    """
    buf = bytearray(256)
    r = Getanalysercomment(
        None,
        addr,
        buf,
        256
    )

    return str(buf.replace('\x00', ''))

def GetProcComment(addr, acall = 0, argonly = 0):
    """
    Get comment generated for a specific Procedure

    Example of comment:
        004017A0  /$  55            PUSH EBP                                 ; breakpoints.004017A0(guessed void)
    """
    buf = bytearray(256)
    r = Getproccomment(
        addr,
        acall,
        buf,
        256,
        argonly
    )

    return str(buf.replace('\x00', ''))

def IsDebuggeeFinished():
    """Is the debugee is finished ?"""
    return cvar.run.status == STAT_FINISHED


# Abstraction

def Run__(status = STAT_RUNNING, pass_exception = 0):
    """
    Run the process, step-in, step-over, whatever
    """
    Run(status, pass_exception)

    # required in order to update the state of the thread registers (retrieved with Threadregisters for example)
    # BTW, not sure it's supposed to be done this way though, I've found that in an OllyDBG2 reverse-engineering session.
    while CheckForDebugEvent() == 1:
        memory.FlushMemoryCache()

def FindMainModule():
    """
    Get a cool structure filled with juicy information concerning
    the process being debugged ; you can find its ImageBase, real ImageBase, etc.
    Check t_module structure definition
    """
    r = Findmainmodule()

    if r is None:
        raise Exception("You haven't loaded any executable file")

    return r

def AddUserComment(address, s):
    """
    Add a user comment at a specific address ; it's like using the shortcut ';'
    """
    return InsertNameW(
        address,
        NM_COMMENT,
        s
    )

def AddUserLabel(address, s):
    """
    Add a user label at a specific address ; it's like using the shortcut ':'
    """
    return InsertNameW(
        address,
        NM_LABEL,
        s
    )

def GetPESections():
    """
    Get all the PE sections of your executable
    Each entry is a t_secthdr
    """
    mod = FindMainModule()
    sects = t_secthdrArray.frompointer(mod.sect)
    sections = []
    for i in range(mod.nsect):
        sections.append(sects[i])
    return sections

def GetEntryPoint():
    """
    Get the address of the entry point of your executable
    """
    mod = FindMainModule()
    return mod.entry

def StepInto():
    """
    Step-into, exactly the same when you hit F7
    """
    Run__(STAT_STEPIN)

def StepOver():
    """
    Step-over, exactly the same when you hit F8
    """
    Run__(STAT_STEPOVER)

def ExecuteUntilRet():
    """
    Execute until RET instruction, exactly the same when you hit ctrl+F9
    """
    Run__(STAT_TILLRET)

def Disass(c, address = 0):
    """
    A high level version of the disass function ; this one is able to disassemble
    more than one instruction.
    """
    sizeof_disassembled_stuff = 0
    complete_disass = []
    sizeof_to_disass = len(c)

    while sizeof_disassembled_stuff != sizeof_to_disass:
        size_current_instruction, disass = Disasm_(c, address + sizeof_disassembled_stuff)
        
        # don't want unicode string
        disass = str(disass)

        # The engine didn't find a valid x86 code
        if disass == '???':
            raise Exception("You have submitted stuff OllyDBG doesn't know how to disassemble")

        # In the other case, we have valid assembly code
        complete_disass.append({
            'text' : disass,
            'size' : size_current_instruction    
        })

        sizeof_disassembled_stuff += size_current_instruction
        c = c[size_current_instruction:]

    return complete_disass

def Assemble__(s, address = 0):
    """
    A high level version of the assemble version ; you can assemble several instructions
    each instruction must be separated by a ';'

    Example: mov eax, 0xdeadbeef ; add eax, 4
    """
    instrs = s.split(';')
    total_size = 0
    code = ''

    for instr in instrs:
        assem, size = Assemble_(instr, address + len(code))

        # your instruction doesn't seem possible to assemble
        if size == 0:
            raise Exception("OllyDBG doesn't know how to assemble this instruction: %s" % s)

        total_size += size
        code += assem

    return (code, total_size)

def FindInstr(instr, address_start = None):
    """
    Find the address of a specific instruction
    """
    if address_start == None:
        address_start = threads.GetEip()

    if memory.IsMemoryExists(address_start) == False:
        return 0

    # now assembleallforms to get the t_asmmod required to call comparecommand
    asmmod, nmodel = '', 0
    try:
        #XXX: fix the ip parameter to be able of finding eip-dependent instruction
        asmmod, nmodel = AssembleAllForms(instr, 0)
    except Exception, e:
        raise(e)

    # get information about the memory block
    mem_info = memory.FindMemory(address_start)

    # now we can call comparecommand
    size_to_dump = mem_info.size - (address_start - mem_info.base)
    offset, found = 0, False

    while offset < size_to_dump and found == False:
        # XXX: maybe its more efficient to read the whole memory block to avoid ReadMemory calls ?
        size_to_read = 16

        # we'll go outside of the boundaries
        if (offset + size_to_read) >= size_to_dump:
            size_to_read = size_to_dump - offset

        code_process = memory.ReadMemory(size_to_read, address_start + offset)

        r = CompareCommand(
            code_process,
            size_to_read,
            address_start + offset,
            asmmod,
            nmodel
        )

        # if this is the same command, we found a match \o/
        if r > 0:
            found = True
        else:
            offset += 1

    if found:
        return address_start + offset

    return 0

def FindHexInPage(s, address_start = None):
    """
    Find hexadecimal values like E9??FF?A??
    The '?' is a wildcard for one nibbles ; that idea comes from the excellent ODBG scripting language

    Note: This function try to find an hexadecimal pattern only in one page of the memory (either in address_start's page
    or in EIP's page)
    """

    def hex_matched(data, s):
        """
        Validate if s match with data
        """
        does_it_matched = True
        idx_data = 0

        for idx in range(0, len(s), 2):
            b_str = s[idx : idx+2]
            byte_to_compare = ord(data[idx_data])

            # have we a wildcard ?
            if '?' in b_str:

                # wildcard on the high nibble
                if b_str[0] == '?' and b_str[1] != '?':
                    low_nibble = (byte_to_compare & 0x0f)
                    if low_nibble != int(b_str[1], 16):
                        does_it_matched = False

                # wildcard on the low nibble
                elif b_str[1] == '?' and b_str[0] != '?':
                    high_nibble = ((byte_to_compare & 0xf0) >> 4)
                    if high_nibble != int(b_str[0], 16):
                        does_it_matched = False
                # wildcard on the entire byte
                else:
                    pass
    
            else:
                b = int(b_str, 16)
                if b != byte_to_compare:
                    does_it_matched = False
            
            idx_data += 1
            if does_it_matched == False:
                break
        
        return does_it_matched

    # ensure we have a multiple of 2 digits
    assert(len(s) % 2 == 0)
    s = s.lower()

    # we only accept hexa digits and the wildcard '?'
    assert(filter(lambda c: c in '0123456789abcdef?', s) == s)

    if address_start == None:
        address_start = threads.GetEip()

    # some memory must be mapped at this address
    if memory.IsMemoryExists(address_start) == False:
        return 0

    # get information about the memory block
    mem_info = memory.FindMemory(address_start)
    
    size_mem_block = mem_info.size - (address_start - mem_info.base)
    offset, found = 0, False
    nb_bytes = len(s) / 2

    while offset < (size_mem_block - nb_bytes) and found == False:
        data = memory.ReadMemory(nb_bytes, address_start + offset)
        if hex_matched(data, s):
            found = True
        else:
            offset += 1

    if found:
        return address_start + offset

    return 0

def display_call_stack(nb_max_frame = 100):
    """
    Walk on the stack & generate a call stack
    """
    frames_info = []
    args = []
    ebp = threads.GetEbp()

    for i in range(nb_max_frame):
        # IsMemoryExists recognizes kernel memory, so we have to manually check it
        if memory.IsMemoryExists(ebp) == False or ebp >= 0x80000000:
            break

        # at EBP we have the SEBP
        sebp = memory.ReadDwordMemory(ebp)
        # and right after the SEIP
        seip = memory.ReadDwordMemory(ebp + 4)

        if sebp == 0 or seip == 0 or memory.IsMemoryExists(sebp) == False or memory.IsMemoryExists(seip) == False:
            break

        symbol = sym.GetSymbolFromAddress(seip)
        frames_info.append({
            'return-address' : seip,
            'address' : sebp + 4,
            'symbol' : symbol if symbol != None else 'no symbol found',
        })

        ebp = sebp

    eip = threads.GetEip()
    print "#%.2d %#.8x : %s" % (len(frames_info), eip, sym.GetSymbolFromAddress(eip))
    
    for i in range(len(frames_info)):
        c = frames_info[i]
        ri = len(frames_info) - i - 1
        print '#%.2d %#.8x : %s (found @%#.8x)' % (ri, c['return-address'], c['symbol'], c['address'])

def display_seh_chain():
    """
    Walk on the stack to find the SEH handlers
    """
    addr_teb = threads.GetCurrentTEB()
    seh_addr = memory.ReadDwordMemory(addr_teb)
    if seh_addr == 0:
        return None

    seh_entries = []

    # This is the last entry if SEH.Next = -1
    while seh_addr != 0xffffffff:
        if memory.IsMemoryExists(seh_addr) == False:
            break

        seh_next, seh_handler = memory.ReadDwordMemory(seh_addr), memory.ReadDwordMemory(seh_addr + 4)

        seh_entries.append({
            'handler' : seh_handler,
            'symbol' : sym.GetSymbolFromAddress(seh_handler),
            'next' : seh_next
        })

        seh_addr = seh_next

    i = 0
    for entry in seh_entries:
        print '#%.2d - Handler: %s (%#.8x) - Next @ %#.8x' % (i, entry['symbol'], entry['handler'], entry['next'])
        i += 1
