# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import ctypes as C
import ctypes.wintypes as wintypes


def convert_cdef_to_pydef(line):
    """\
convert_cdef_to_pydef(line_from_c_header_file) -> python_tuple_string
'DWORD  var_name[LENGTH];' -> '("var_name", DWORD*LENGTH)'

doesn't work for all valid c/c++ declarations"""
    l = line[:line.find(';')].split()
    if len(l) != 2:
        return None
    type_ = l[0]
    name = l[1]
    i = name.find('[')
    if i != -1:
        name, brac = name[:i], name[i:][1:-1]
        return '("%s", %s*%s)' % (name, type_, brac)
    return '("%s", %s)' % (name, type_)


def convert_cdef_to_structure(cdef, name, data_dict=C.__dict__):
    """\
convert_cdef_to_structure(struct_definition_from_c_header_file)
  -> python class derived from ctypes.Structure

limited support for c/c++ syntax"""
    py_str = '[\n'
    for line in cdef.split('\n'):
        field = convert_cdef_to_pydef(line)
        if not field is None:
            py_str += ' ' * 4 + field + ',\n'
    py_str += ']\n'

    pyarr = eval(py_str, data_dict)

    class ret_val(C.Structure):
        _fields_ = pyarr

    ret_val.__name__ = name
    ret_val.__module__ = None
    return ret_val

winnt = (
    ('IMAGE_DOS_HEADER', """\
    WORD   e_magic;
    WORD   e_cblp;
    WORD   e_cp;
    WORD   e_crlc;
    WORD   e_cparhdr;
    WORD   e_minalloc;
    WORD   e_maxalloc;
    WORD   e_ss;
    WORD   e_sp;
    WORD   e_csum;
    WORD   e_ip;
    WORD   e_cs;
    WORD   e_lfarlc;
    WORD   e_ovno;
    WORD   e_res[4];
    WORD   e_oemid;
    WORD   e_oeminfo;
    WORD   e_res2[10];
    LONG   e_lfanew;
"""),

    ('IMAGE_FILE_HEADER', """\
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
"""),

    ('IMAGE_DATA_DIRECTORY', """\
    DWORD   VirtualAddress;
    DWORD   Size;
"""),

    ('IMAGE_OPTIONAL_HEADER32', """\
    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;
    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
""",
     {'IMAGE_NUMBEROF_DIRECTORY_ENTRIES': 16}
    ),

    ('IMAGE_NT_HEADERS', """\
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
"""),

    ('IMAGE_OPTIONAL_HEADER64', """\
    WORD        Magic;
    BYTE        MajorLinkerVersion;
    BYTE        MinorLinkerVersion;
    DWORD       SizeOfCode;
    DWORD       SizeOfInitializedData;
    DWORD       SizeOfUninitializedData;
    DWORD       AddressOfEntryPoint;
    DWORD       BaseOfCode;
    c_ulonglong   ImageBase;
    DWORD       SectionAlignment;
    DWORD       FileAlignment;
    WORD        MajorOperatingSystemVersion;
    WORD        MinorOperatingSystemVersion;
    WORD        MajorImageVersion;
    WORD        MinorImageVersion;
    WORD        MajorSubsystemVersion;
    WORD        MinorSubsystemVersion;
    DWORD       Win32VersionValue;
    DWORD       SizeOfImage;
    DWORD       SizeOfHeaders;
    DWORD       CheckSum;
    WORD        Subsystem;
    WORD        DllCharacteristics;
    c_ulonglong   SizeOfStackReserve;
    c_ulonglong   SizeOfStackCommit;
    c_ulonglong   SizeOfHeapReserve;
    c_ulonglong   SizeOfHeapCommit;
    DWORD       LoaderFlags;
    DWORD       NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
    """,
     {'IMAGE_NUMBEROF_DIRECTORY_ENTRIES': 16}),

    ('IMAGE_NT_HEADERS64', """
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
    """),

    ('IMAGE_EXPORT_DIRECTORY', """\
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Name;
    DWORD   Base;
    DWORD   NumberOfFunctions;
    DWORD   NumberOfNames;
    DWORD   AddressOfFunctions;
    DWORD   AddressOfNames;
    DWORD   AddressOfNameOrdinals;
"""),
)

#Construct python ctypes.Structures from above definitions
data_dict = dict(wintypes.__dict__)
for definition in winnt:
    name = definition[0]
    def_str = definition[1]
    if len(definition) == 3:
        data_dict.update(definition[2])
    type_ = convert_cdef_to_structure(def_str, name, data_dict)
    data_dict[name] = type_
    globals()[name] = type_

    ptype = C.POINTER(type_)
    pname = 'P'+name
    data_dict[pname] = ptype
    globals()[pname] = ptype

del data_dict
del winnt



IMAGE_DOS_SIGNATURE = 0x5A4D
IMAGE_NT_SIGNATURE = 0x00004550
IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16
IMAGE_ORDINAL_FLAG = 0x80000000L
OPTIONAL_HEADER_MAGIC_PE = 0x10b
OPTIONAL_HEADER_MAGIC_PE_PLUS = 0x20b

IMAGE_SIZEOF_SHORT_NAME = 8

IMAGE_DIRECTORY_ENTRY_EXPORT = 0
IMAGE_DIRECTORY_ENTRY_IMPORT = 1
IMAGE_DIRECTORY_ENTRY_RESOURCE = 2
IMAGE_DIRECTORY_ENTRY_EXCEPTION = 3
IMAGE_DIRECTORY_ENTRY_SECURITY = 4
IMAGE_DIRECTORY_ENTRY_BASERELOC = 5
IMAGE_DIRECTORY_ENTRY_DEBUG = 6
IMAGE_DIRECTORY_ENTRY_COPYRIGHT = 7
IMAGE_DIRECTORY_ENTRY_ARCHITECTURE = 7
IMAGE_DIRECTORY_ENTRY_GLOBALPTR = 8
IMAGE_DIRECTORY_ENTRY_TLS = 9
IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG = 10
IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT = 11
IMAGE_DIRECTORY_ENTRY_IAT = 12
IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT = 13
IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR = 14

IMAGE_ORDINAL_FLAG32 = 0x80000000
IMAGE_FILE_MACHINE_I386 = 0x014c
IMAGE_FILE_MACHINE_AMD64 = 0x8664

PAGE_GUARD = 0x100
TH32CS_SNAPMODULE = 0x00000008

MEM_COMMIT = 0x1000
MEM_FREE = 0x10000
MEM_RESERVE = 0x2000
MEM_RELEASE = 0x8000

PAGE_EXECUTE_READWRITE = 0x40


class IMAGE_SECTION_HEADER_MISC(C.Union):
    _fields_ = [('PhysicalAddress', C.c_ulong),
                ('VirtualSize', C.c_ulong)]


class IMAGE_SECTION_HEADER(C.Structure):
    _fields_ = [('Name', C.c_ubyte * IMAGE_SIZEOF_SHORT_NAME),
                ('Misc', IMAGE_SECTION_HEADER_MISC),
                ('VirtualAddress', C.c_ulong),
                ('SizeOfRawData', C.c_ulong),
                ('PointerToRawData', C.c_ulong),
                ('PointerToRelocations', C.c_ulong),
                ('PointerToLinenumbers', C.c_ulong),
                ('NumberOfRelocations', C.c_uint16),
                ('NumberOfLinenumbers', C.c_uint16),
                ('Characteristics', C.c_ulong)]


class IMAGE_IMPORT_DESCRIPTOR_MISC(C.Union):
    _fields_ = [('Characteristics', C.c_ulong),  # 0 for terminating null import descriptor
        #  RVA to original unbound IAT (PIMAGE_THUNK_DATA)
                ('OriginalFirstThunk', C.c_ulong)]


class IMAGE_IMPORT_DESCRIPTOR(C.Structure):
    _fields_ = [('Misc', IMAGE_IMPORT_DESCRIPTOR_MISC),
        # 0 if not bound,
        #  -1 if bound, and real date/time stamp
        #     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
        #  O.W. date/time stamp of DLL bound to (Old BIND)
                ('TimeDateStamp', C.c_ulong),
                ('ForwarderChain', C.c_ulong),  # -1 if no forwarders
                ('Name', C.c_ulong),
        # RVA to IAT (if bound this IAT has actual addresses)
                ('FirstThunk', C.c_ulong)]


class IMAGE_IMPORT_BY_NAME(C.Structure):
    _fields_ = [('Hint', wintypes.WORD),
                ('Name', wintypes.BYTE)]


class IMAGE_THUNK_DATA(C.Union):
    _fields_ = [('ForwarderString', C.c_ulong),  # PBYTE
                ('Function', C.c_ulong),  # PDWORD
                ('Ordinal', C.c_ulong),
                ('AddressOfData', C.c_ulong)]  # PIMAGE_IMPORT_BY_NAME


class IMAGE_THUNK_DATA64(C.Union):
    _fields_ = [('ForwarderString', C.c_ulonglong),
                ('Function', C.c_ulonglong),
                ('Ordinal', C.c_ulonglong),
                ('AddressOfData', C.c_ulonglong)]


class MEMORY_BASIC_INFORMATION(C.Structure):
    _fields_ = [('BaseAddress', C.c_long),
                ('AllocationBase', C.c_long),
                ('AllocationProtect', C.c_long),
                ('AllocationProtect', C.c_long),
                ('RegionSize', C.c_long),
                ('State', C.c_long),
                ('Protect', C.c_long),
                ('Type', C.c_long)]


class MEMORY_BASIC_INFORMATION64(C.Structure):
    _fields_ = [('BaseAddress', C.c_ulonglong),
                ('AllocationBase', C.c_ulonglong),
                ('AllocationProtect', C.c_long),
                ('__alignment1', C.c_long),
                ('RegionSize', C.c_ulonglong),
                ('State', C.c_long),
                ('Protect', C.c_long),
                ('Type', C.c_long),
                ('__alignment2', C.c_long)]


class MODULEENTRY32(C.Structure):
    _fields_ = [('dwSize', C.c_long),
                ('th32ModuleID', C.c_long),
                ('th32ProcessID', C.c_long),
                ('GlblcntUsage', C.c_long),
                ('ProccntUsage', C.c_long),
                ('modBaseAddr', C.c_void_p),
                ('modBaseSize', C.c_long),
                ('hModule', C.c_void_p),
                ('szModule', C.c_char * 256),
                ('szExePath', C.c_char * 260)]
