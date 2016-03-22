# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import ctypes as C
import sys
import traceback

import pehelper_decl as D


class PEHelper(object):
    def __init__(self, base, modname, data):
        super(PEHelper, self).__init__()
        self._base = base
        self._modname = modname
        self._data = (C.c_ubyte * len(data)).from_buffer_copy(data)

        self._parsed = False
        self._exports = list()
        self._ea_to_long_name = dict()

        # internal
        self._dos_header = None
        self._nt_headers = None
        self._opt_header = None
        self._exports_dir_entry = None
        self._export_dir = None
        self._imports_dir_entry = None
        self._imports_descriptors = None
        self._sections_info = None
        self._is_x64 = None

    def check_rva_inside_buffer(self, p):
        if p < 0 or p >= len(self._data):
            raise Exception('Wrong RVA 0x%08X, image size: 0x%08X' % (p, len(self._data)))

    def cast_rva(self, rva, type_, isPtr):
        self.check_rva_inside_buffer(rva)
        if isPtr and not isinstance(type_._type_, basestring):
            s = C.sizeof(type_._type_)
        else:
            s = C.sizeof(type_)
        if rva + s >= self._base + len(self._data):
            raise Exception(('Invalid RVA and size of type: (rva: 0x%08X, size: 0x%08X),' +
                             ' out of module image (base: 0x%08X, size: 0x%08X') %
                            (rva, s, self._base, len(self._data)))
        return C.cast(C.c_void_p(C.addressof(self._data) + rva), type_)

    def _parse_dos_header(self):
        self._dos_header = self.cast_rva(0, D.PIMAGE_DOS_HEADER, True)[0]
        if self._dos_header.e_magic != D.IMAGE_DOS_SIGNATURE:
            print >> sys.stderr, 'Invalid PE header (invalid DOS magic)'
            raise Exception('Invalid PE header (invalid DOS magic)')

    def _parse_nt_headers(self):
        self._nt_headers = self.cast_rva(self._dos_header.e_lfanew, D.PIMAGE_NT_HEADERS, True)[0]
        if self._nt_headers.Signature != D.IMAGE_NT_SIGNATURE:
            print >> sys.stderr, 'Invalid PE header (invalid nt signature)'
            raise Exception('Invalid PE header (invalid nt signature)')
        if self._nt_headers.FileHeader.Machine not in (D.IMAGE_FILE_MACHINE_I386, D.IMAGE_FILE_MACHINE_AMD64):
            print >> sys.stderr, 'Invalid PE header (Invalid machine type, supported only i386 and amd64)'
            raise Exception('Invalid PE header (Invalid machine type, supported only i386 and amd64)')
        self._is_x64 = self._nt_headers.FileHeader.Machine == D.IMAGE_FILE_MACHINE_AMD64
        if self._is_x64:
            self._nt_headers = self.cast_rva(self._dos_header.e_lfanew, D.PIMAGE_NT_HEADERS64, True)[0]
        self._opt_header = self._nt_headers.OptionalHeader
        if not self._is_x64 and self._opt_header.Magic != D.OPTIONAL_HEADER_MAGIC_PE:
            print >> sys.stderr, 'Invalid PE header (invalid optional header signature)'
            raise Exception('Invalid PE header (invalid optional header signature)')
        if self._is_x64 and self._opt_header.Magic != D.OPTIONAL_HEADER_MAGIC_PE_PLUS:
            print >> sys.stderr, 'Invalid PE header (invalid optional header signature)'
            raise Exception('Invalid PE header (invalid optional header signature)')

        if self._opt_header.NumberOfRvaAndSizes <= 0 or self._opt_header.NumberOfRvaAndSizes > 0x10:
            print >> sys.stderr, 'Invalid PE header (NumberOfRvaAndSizes has invalid value)'
            raise Exception('Invalid PE header (NumberOfRvaAndSizes has invalid value)')

    def _parse_exports_dir(self):
        self._exports_dir_entry = self._opt_header.DataDirectory[D.IMAGE_DIRECTORY_ENTRY_EXPORT]
        if not self._exports_dir_entry:
            return
        self._export_dir = self.cast_rva(self._exports_dir_entry.VirtualAddress, D.PIMAGE_EXPORT_DIRECTORY, True)[0]
        if self._exports_dir_entry.Size == 0 or self._exports_dir_entry.Size > 0x100000:
            return
        nNames = self._export_dir.NumberOfNames
        nFuncs = self._export_dir.NumberOfFunctions
        if nNames <= 0 or nNames > 0x100000 or nFuncs <= 0 or nFuncs > 0x100000:
            # bad PE header
            return

        self._exports = list()
        self._ea_to_long_name = dict()
        if nNames > 0:
            PNamesType = C.POINTER(C.c_uint * nNames)
            POrdinalsType = C.POINTER(C.c_ushort * nNames)
            PFuncsType = C.POINTER(C.c_uint * nFuncs)

            names = self.cast_rva(self._export_dir.AddressOfNames, PNamesType, True)[0]
            ordinals = self.cast_rva(self._export_dir.AddressOfNameOrdinals, POrdinalsType, True)[0]
            funcs = self.cast_rva(self._export_dir.AddressOfFunctions, PFuncsType, True)[0]
            ordinalBase = self._export_dir.Base
            self._exports = [{'ea': self._base + x,
                              'ord': ordinalBase + i,
                              'name': '#%u' % (ordinalBase + i)} for i, x in enumerate(funcs)]

            for i, rva in enumerate(names):
                name = self.cast_rva(rva, C.c_char_p, True).value
                nameOrdinal = ordinals[i]
                if nameOrdinal < 0 or nameOrdinal >= len(funcs):
                    continue

                self._exports[nameOrdinal]['name'] = name
            for exp in self._exports:
                self._ea_to_long_name[exp['ea']] = '%s.%s' % (self._modname, exp['name'])
            #print self._exports

    # def _parse_imports_dir(self):
    #     self._imports_dir_entry = self._opt_header.DataDirectory[D.IMAGE_DIRECTORY_ENTRY_IMPORT]
    #     if not self._imports_dir_entry:
    #         return
    #     count = self._imports_dir_entry.Size / C.sizeof(D.IMAGE_IMPORT_DESCRIPTOR) - 1
    #     self._imports_descriptors = self.cast_rva(self._imports_dir_entry.VirtualAddress,
    #                                               C.POINTER(D.IMAGE_IMPORT_DESCRIPTOR * count), True)[0]

    def _parse_sections(self):
        self._sections_info = list()
        numSections = self._nt_headers.FileHeader.NumberOfSections
        offset = self._dos_header.e_lfanew + C.sizeof(D.IMAGE_NT_HEADERS if not self._is_x64 else D.IMAGE_NT_HEADERS64)
        if numSections > 1000:
            raise Exception('Too much sections: %s' % numSections)
        PSections = C.POINTER(D.IMAGE_SECTION_HEADER * numSections)
        sections = self.cast_rva(offset, PSections, True)[0]
        for sec in sections:
            name = bytearray(sec.Name)
            if '\x00' in name:
                name = name[:name.index('\x00')]
            section = {
                'name': str(name),
                'va': sec.VirtualAddress,
                'v_size': sec.Misc.VirtualSize,
                'raw': sec.PointerToRawData,
                'raw_size': sec.SizeOfRawData,
                'p_rel': sec.PointerToRelocations,
                'n_rel': sec.NumberOfRelocations,
                'ch': sec.Characteristics
            }
            self._sections_info.append(section)

    def parse_headers(self, strict=False):
        try:
            self._parse_dos_header()
            self._parse_nt_headers()

            # self._parse_imports_dir()
            self._parse_sections()

            self._parse_exports_dir()
            self._parsed = True
            return True
        except Exception as e:
            print >> sys.stderr, 'Exception: %r\r\n%s' % (e, traceback.format_exc().replace('\n', '\r\n'))
        return False

    def get_exports(self):
        if not self._parsed:
            self.parse_headers()
        return self._exports

    def get_ea_to_longname_map(self):
        if not self._parsed:
            self.parse_headers()
        return self._ea_to_long_name

    def get_sections(self):
        if not self._parsed:
            self.parse_headers()
        return self._sections_info


def perform_test():
    with open('d:\\pe_test.bin', 'rb') as f:
        mem = bytearray(f.read())
    ph = PEHelper(0x6f010000, 'comctl32', mem)
    ph.get_exports()


if __name__ == '__main__':
    perform_test()
