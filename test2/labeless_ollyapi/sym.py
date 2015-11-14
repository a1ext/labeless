#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#    sym.py - High level API to play with the Symbol API
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
from ctypes import *
import threads

# Structures

MAX_SYM_NAME = 2000
MAX_PATH = 260

class symbol_info_t(Structure):
    """
    Contains symbol information.
    Size: 88bytes
    """
    _pack_ = 8
    _fields_ = [
        # The size of the structure, in bytes. This member must be set to sizeof(SYMBOL_INFO).
        # Note that the total size of the data is the SizeOfStruct + (MaxNameLen - 1) * sizeof(TCHAR).
        # The reason to subtract one is that the first character in the name is accounted for in the size of the structure.
        ('SizeOfStruct', c_ulong),

        # A unique value that identifies the type data that describes the symbol. This value does not persist between sessions.
        ('TypeIndex', c_ulong),

        # This member is reserved for system use.
        ('Reserved', c_longlong * 2),

        # The unique value for the symbol. The value associated with a symbol is not guaranteed to be the same each time you run the process.
        # For PDB symbols, the index value for a symbol is not generated until the symbol is enumerated or retrieved through a search by name or address.
        # The index values for all CodeView and COFF symbols are generated when the symbols are loaded.
        ('Index', c_ulong),

        # The symbol size, in bytes.
        # This value is meaningful only if the module symbols are from a pdb file; otherwise, this value is typically zero and should be ignored.
        ('Size', c_ulong),

        # The base address of the module that contains the symbol.
        ('ModBase', c_longlong),

        ('Flags', c_ulong),

        # The value of a constant.
        ('Value', c_longlong),

        # The virtual address of the start of the symbol.
        ('Address', c_longlong),

        # The register.
        ('Register', c_ulong),

        # The DIA scope.
        # For more information, see the Debug Interface Access SDK in the Visual Studio documentation. 
        # (This resource may not be available in some languages and countries.)
        ('Scope', c_ulong),

        # The PDB classification.
        # These values are defined in Dbghelp.h in the SymTagEnum enumeration type.
        ('Tag', c_ulong),

        # The length of the name, in characters, not including the null-terminating character.
        # addr_array = addressof(struct) + sizeof(ULONG) + sizeof(ULONG)
        # s = string_at(addr_array)
        ('NameLen', c_ulong),

        # The size of the Name buffer, in characters.
        # If this member is 0, the Name member is not used.
        ('MaxNameLen', c_ulong),

        # The name of the symbol.
        # The name can be undecorated if the SYMOPT_UNDNAME option is used with the SymSetOptions function.
        ('Name', c_char * 1)
    ]

symbol_info_p = POINTER(symbol_info_t)

class guid_t(Structure):
    """
    GUIDs identify objects such as interfaces, manager entry-point vectors (EPVs), and class objects.
    A GUID is a 128-bit value consisting of one group of 8 hexadecimal digits,
    followed by three groups of 4 hexadecimal digits each, followed by one group of 12 hexadecimal digits.
    The following example GUID shows the groupings of hexadecimal digits in a GUID: 6B29FC40-CA47-1067-B31D-00DD010662DA

    Size: 16bytes
    """
    _pack_ = 4
    _fields_ = [
        ('Data1', c_ulong),
        ('Data2', c_ushort),
        ('Data3', c_ushort),
        ('Data4', c_char * 8)
    ]

class imagehlp_module64_t(Structure):
    """
    Contains module information.
    Size: 1672
    """
    _pack_ = 8
    _fields_ = [
        # The size of the structure, in bytes.
        # The caller must set this member to sizeof(IMAGEHLP_MODULE64).
        ('SizeOfStruct', c_ulong),

        # The base virtual address where the image is loaded.
        ('BaseOfImage', c_longlong),

        # The size of the image, in bytes.
        ('ImageSize', c_ulong),

        # The date and timestamp value.
        # The value is represented in the number of seconds elapsed since midnight (00:00:00), January 1, 1970, 
        # Universal Coordinated Time, according to the system clock.
        # The timestamp can be printed using the C run-time (CRT) function ctime.
        ('TimeDateStamp', c_ulong),

        # The checksum of the image.
        # This value can be zero.
        ('CheckSum', c_ulong),

        # The number of symbols in the symbol table.
        # The value of this parameter is not meaningful when SymPdb is specified as the value of the SymType parameter.
        ('NumSyms', c_ulong),

        # The type of symbols that are loaded.
        ('SymType', c_int),

        # The module name.
        ('ModuleName', c_char * 32),

        # The image name. The name may or may not contain a full path.
        ('ImageName', c_char * 256),

        # The full path and file name of the file from which symbols were loaded.
        ('LoadedImageName', c_char * 256),

        # The full path and file name of the .pdb file.
        ('LoadedPdbName', c_char * 256),

        # The signature of the CV record in the debug directories.
        ('CVSig', c_ulong),

        # The contents of the CV record.
        ('CVData', c_char * (MAX_PATH * 3)),

        # The PDB signature.
        ('PdbSig', c_ulong),

        # The PDB signature (Visual C/C++ 7.0 and later)
        ('PdbSig70', guid_t),

        # The DBI age of PDB.
        ('PdbAge', c_ulong),

        # A value that indicates whether the loaded PDB is unmatched.
        ('PdbUnmatched', c_long),

        # A value that indicates whether the loaded DBG is unmatched.
        ('DbgUnmatched', c_long),

        # A value that indicates whether line number information is available.
        ('LineNumbers', c_long),

        # A value that indicates whether symbol information is available.
        ('GlobalSymbols', c_long),

        # A value that indicates whether type information is available.
        ('TypeInfo', c_long),

        # A value that indicates whether the .pdb supports the source server.
        ('SourceIndexed', c_long),

        # A value that indicates whether the module contains public symbols.
        ('Publics', c_long),
    ]

imagehlp_module64_p = POINTER(imagehlp_module64_t)

c_longlong_p = POINTER(c_longlong)

# Wrapper

def resolve_api(n, mod):
    """
    Retrieve dynamically the function address exported
    by OllyDbg
    """
    addr = windll.kernel32.GetProcAddress(
        windll.kernel32.GetModuleHandleA(mod),
        n
    )

    assert(addr != 0)
    return addr

# XXX: ctypes.wintypes doesn't exist in python 2.6
# BOOL WINAPI SymInitialize(
#   _In_      HANDLE hProcess,
#   _In_opt_  PCTSTR UserSearchPath,
#   _In_      BOOL fInvadeProcess
# );

# In [13]: wintypes.BOOL
# Out[13]: ctypes.c_long
# In [14]: wintypes.HANDLE
# Out[14]: ctypes.c_void_p
# In [15]: wintypes.LPCSTR
# Out[15]: ctypes.c_char_p
Syminitialize_TYPE = WINFUNCTYPE(c_long, c_void_p, c_char_p, c_long)
Syminitialize = Syminitialize_TYPE(resolve_api('SymInitialize', 'dbghelp.dll'))

# BOOL WINAPI SymFromAddr(
#   _In_       HANDLE hProcess,
#   _In_       DWORD64 Address,
#   _Out_opt_  PDWORD64 Displacement,
#   _Inout_    PSYMBOL_INFO Symbol
# );
Symfromaddr_TYPE = WINFUNCTYPE(c_bool, c_void_p, c_longlong, c_longlong_p, symbol_info_p)
Symfromaddr = Symfromaddr_TYPE(resolve_api('SymFromAddr', 'dbghelp.dll'))

# BOOL WINAPI SymGetModuleInfo64(
#   _In_   HANDLE hProcess,
#   _In_   DWORD64 dwAddr,
#   _Out_  PIMAGEHLP_MODULE64 ModuleInfo
# );
Symgetmoduleinfo64_TYPE = WINFUNCTYPE(c_bool, c_void_p, c_longlong, imagehlp_module64_p)
Symgetmoduleinfo64 = Symgetmoduleinfo64_TYPE(resolve_api('SymGetModuleInfo64', 'dbghelp.dll'))

def SymInitialize(hProcess, UserSearchPath = None, fInvadeProcess = True):
    """
    Initializes the symbol handler for a process.

    Note: OllyDBG seems to call this function, thus you shouldn't call it.
    """
    return Syminitialize(
        hProcess,
        UserSearchPath,
        fInvadeProcess
    )

def SymFromAddr(hProcess, address):
    """
    Retrieves symbol information for the specified address.
    """
    displacement = c_longlong(0)

    # A pointer to a SYMBOL_INFO structure that provides information about the symbol.
    # The symbol name is variable in length; therefore this buffer must be large enough to hold the name stored at the end of the SYMBOL_INFO structure.
    # Be sure to set the MaxNameLen member to the number of bytes reserved for the name.
    buf = create_string_buffer(sizeof(symbol_info_t) + (MAX_SYM_NAME * sizeof(c_char)))
    p_symbol = cast(buf, symbol_info_p)
    p_symbol.contents.SizeOfStruct = c_ulong(sizeof(symbol_info_t))
    p_symbol.contents.MaxNameLen = MAX_SYM_NAME

    r = Symfromaddr(
        c_void_p(hProcess),
        c_longlong(address),
        c_longlong_p(displacement),
        p_symbol
    )

    if r == False:
        return None

    # -4 because of the pad appended after our structure
    addr_s = addressof(buf) + sizeof(symbol_info_t) - 4
    s = string_at(addr_s)

    return {
        'struct' : p_symbol.contents,
        's' : s,
        'displacement' : displacement
    }

def SymGetModuleInfo64(hProcess, address):
    """
    Retrieves the module information of the specified module.
    """
    
    img = imagehlp_module64_t()
    img.SizeOfStruct = c_ulong(sizeof(imagehlp_module64_t))

    r = Symgetmoduleinfo64(
        c_void_p(hProcess),
        c_longlong(address),
        imagehlp_module64_p(img)
    )

    if r == 0:
        return None
    
    return img

def DecodeAddress(addr):
    """
    Obtain symbol from OllyDBG via an address

    Note: It works only if address is the *exact* address of the function
    I mean if you're trying to get the symbol of function+1, it won't retrieve anything
    In this case you have to use DecodeRelativeOffset

    Example:
    DecodeAddress(0x31337) = binary.function
    """
    buf = bytearray(256)
    r = Decodeaddress(
        addr,
        0,
        0x20400,
        buf,
        256,
        None
    )

    if r <= 0:
        return None

    return str(buf.replace('\x00', ''))

def DecodeRelativeOffset(addr):
    """
    """
    buf = bytearray(256)
    r = Decoderelativeoffset(
        addr,
        0x20400,
        buf,
        100
    )

    if r <= 0:
        return None

    return str(buf.replace('\x00', ''))

# Abstraction


def GetSymbolFromAddressMS(address):
    """
    Retrieve symbol information from an address via the Windows Symbol API
    
    Example:
    GetSymbolFromAddressMS(0x778de752) = ntdll!RtlAnsiStringToUnicodeString+0x0000007d
    """
    handle_process = threads.GetProcessHandle()
    address_info = SymFromAddr(handle_process, address)
    s = None
    
    if address_info != None:
        symbol_name, offset = address_info['s'], address_info['displacement'].value
        module_info = SymGetModuleInfo64(handle_process, address)

        if module_info != None:
            s = '%s.%s+%#.8x' % (module_info.ModuleName, symbol_name, offset)
        else:
            s = '%s+%#.8x' % (symbol_name, offset)

    return s

def GetSymbolFromAddressOlly(address):
    """
    Retrieve symbol information from an address via the OllyDBG API
    
    Example:
    GetSymbolFromAddressOlly(0x778de752) =
    """
    s = DecodeRelativeOffset(address)
    if s == None:
        s = DecodeAddress(address)

    return s

def GetSymbolFromAddress(address):
    """
    Try to obtain a symbol via, first the MS API,
    and if it didn't succeed via the OllyDBG API
    """
    s = GetSymbolFromAddressMS(address)
    if s == None:
        s = GetSymbolFromAddressOlly(address)

    return s
