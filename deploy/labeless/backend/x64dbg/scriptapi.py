# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import ctypes as C
from . import api

__MAX_PATH = 260


class Assembler(object):
    @staticmethod
    def Assemble(addr, dest, instruction):
        """
        :param duint addr:
        :param unsigned char * dest:
        :param char const * instruction:
        :return int size or None"""
        p_size = api.new_intp()
        try:
            if api.Assembler_Assemble(addr, dest, p_size, instruction):
                return api.intp_value(p_size)
        finally:
            api.delete_intp(p_size)


    @staticmethod
    def AssembleEx(addr, dest, instruction):
        """
        :param duint addr:
        :param unsigned char * dest:
        :param char const * instruction:
        :return int size if OK otherwise str error or None """
        size = api.new_intp()
        try:
            dest = C.create_string_buffer(dest)
            error = C.create_string_buffer(api.MAX_ERROR_SIZE)
            if not api.Assembler_AssembleEx(addr, dest, size, instruction, error):
                return error.value or None
            return api.intp_value(size)
        finally:
            api.delete_intp(size)


    @staticmethod
    def AssembleMem(addr, instruction):
        """
        :param duint addr:
        :param char const * instruction:
        :return bool"""
        return api.Assembler_AssembleMem(addr, instruction)

    @staticmethod
    def AssembleMemEx(addr, instuction, fillnop):
        """
        :param duint addr:
        :param char const * instruction:
        :param bool fillnop:
        :return int size if OK, str error or None if failed"""
        p_size = api.new_intp()
        try:
            error = C.create_string_buffer(api.MAX_ERROR_SIZE)
            if api.Assembler_AssembleMemEx(addr, instuction, p_size, error, fillnop):
                return api.intp_value(p_size)
            return error.value or None
        finally:
            api.delete_intp(p_size)


class Bookmark(object):
    @staticmethod
    def Set(manual=False):
        """
        Bookmark_Set(duint addr, bool manual=False) -> bool
        Bookmark_Set(duint addr) -> bool
        """
        return api.Bookmark_Set(manual)

    @staticmethod
    def SetByBookmarkInfo(*args):
        """Bookmark_SetByBookmarkInfo(BookmarkInfo info) -> bool"""
        return api.Bookmark_SetByBookmarkInfo(*args)

    @staticmethod
    def Get(addr):
        """Bookmark_Get(duint addr) -> bool"""
        return api.Bookmark_Get(addr)

    @staticmethod
    def GetInfo(addr):
        """Bookmark_GetInfo(duint addr) -> BookmarkInfo() or None"""
        info = api.BookmarkInfo()
        if api.Bookmark_GetInfo(addr, info):
            return info

    @staticmethod
    def Delete(addr):
        """Bookmark_Delete(duint addr) -> bool"""
        return api.Bookmark_Delete(addr)

    @staticmethod
    def DeleteRange(start, end):
        """Bookmark_DeleteRange(duint start, duint end)"""
        return api.Bookmark_DeleteRange(start, end)

    @staticmethod
    def Clear(cls):
        """Bookmark_Clear()"""
        return api.Bookmark_Clear()

    @staticmethod
    def GetList():
        """
        :return list({'mod': str, 'rva': long, 'manual': bool}) or None"""
        info = api.ListInfo()
        if not api.Bookmark_GetList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.BookmarkInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'mod': str(item.mod).replace('\0', ''),
                    'rva': long(item.rva),
                    'manual': item.manual
                })

        finally:
            api.BridgeFree(info.data)

        return rv


class Comment(object):
    @staticmethod
    def Set(addr, text, manual=False):
        """
        :param duint addr:
        :param char const * text:
        :param bool manual=False
        :return bool
        """
        return api.Comment_Set(addr, text, manual)

    @staticmethod
    def ByCommentInfo(*args):
        """
        :param CommentInfo info:
        :return bool"""
        return api.Comment_ByCommentInfo(*args)

    @staticmethod
    def Get(addr):
        """
        :param duint addr:
        :return str or None"""
        rv = C.create_string_buffer(api.MAX_COMMENT_SIZE)
        if api.Comment_Get(addr, rv):
            return rv.value

    @staticmethod
    def GetInfo(addr):
        """
        :param duint addr:
        :return CommentInfo() or None"""
        info = api.CommentInfo()
        if api.Comment_GetInfo(addr, info):
            return info

    @staticmethod
    def Delete(addr):
        """
        :param duint addr:
        :return bool"""
        return api.Comment_Delete(addr)

    @staticmethod
    def DeleteRange(*args):
        """
        :param duint start:
        :param duint end"""
        return api.Comment_DeleteRange(*args)

    @staticmethod
    def Clear():
        return api.Comment_Clear()

    @staticmethod
    def GetList():
        """:return list({'mod': str, 'rva': long, 'text': str, 'manual': bool}) or None"""
        info = api.ListInfo()
        if not api.Comment_GetList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.CommentInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'mod': str(item.mod).replace('\0', ''),
                    'rva': long(item.rva),
                    'text': str(item.text).replace('\0', ''),
                    'manual': item.manual
                })

        finally:
            api.BridgeFree(info.data)

        return rv


class Debug(object):
    @staticmethod
    def Debug_Wait():
        """Debug_Wait()"""
        return api.Debug_Wait()

    @staticmethod
    def Debug_Run():
        """Debug_Run()"""
        return api.Debug_Run()

    @staticmethod
    def Debug_Pause():
        """Debug_Pause()"""
        return api.Debug_Pause()

    @staticmethod
    def Debug_Stop():
        """Debug_Stop()"""
        return api.Debug_Stop()

    @staticmethod
    def Debug_StepIn():
        """Debug_StepIn()"""
        return api.Debug_StepIn()

    @staticmethod
    def Debug_StepOver():
        """Debug_StepOver()"""
        return api.Debug_StepOver()

    @staticmethod
    def Debug_StepOut():
        """Debug_StepOut()"""
        return api.Debug_StepOut()

    @staticmethod
    def Debug_SetBreakpoint(address):
        """
        :param duint address:
        :return bool"""
        return api.Debug_SetBreakpoint(address)

    @staticmethod
    def Debug_DeleteBreakpoint(address):
        """
        :param duint address:
        :return bool"""
        return api.Debug_DeleteBreakpoint(address)

    @staticmethod
    def Debug_SetHardwareBreakpoint(address, type_=api.HardwareExecute):
        """
        :param duint address:
        :param Script::Debug::HardwareType type=HardwareExecute
        :return bool"""
        return api.Debug_SetHardwareBreakpoint(address, type_)

    @staticmethod
    def Debug_DeleteHardwareBreakpoint(address):
        """
        :param duint address:
        :return bool"""
        return api.Debug_DeleteHardwareBreakpoint(address)


# TODO: class Flag


class Function(object):
    @staticmethod
    def Function_Add(start, end, manual, instructionCount=0):
        """
        :param duint start:
        :param duint end:
        :param bool manual:
        :param duint instructionCount=0
        :return bool"""
        return api.Function_Add(start, end, manual, instructionCount)

    @staticmethod
    def Function_AddByFuncInfo(info):
        """
        :param FunctionInfo info:
        :return bool"""
        return api.Function_AddByFuncInfo(info)

    @staticmethod
    def Function_Get(*args):
        """
        :param duint addr:
        :param duint * start=None
        :param duint * end=None
        :param duint * instructionCount=None
        :return bool"""
        return api.Function_Get(*args)

    @staticmethod
    def Function_GetInfo(addr):
        """
        :param duint addr:
        :return FunctionInfo() or None"""
        rv = api.FunctionInfo()
        if api.Function_GetInfo(addr, rv):
            return rv

    @staticmethod
    def Function_Overlaps(start, end):
        """
        :param duint start:
        :param duint end:
        :return bool"""
        return api.Function_Overlaps(start, end)

    @staticmethod
    def Function_Delete(address):
        """
        :param duint address:
        :return bool"""
        return api.Function_Delete(address)

    @staticmethod
    def Function_DeleteRange(start, end):
        """
        :param duint start:
        :param duint end"""
        api.Function_DeleteRange(start, end)

    @staticmethod
    def Function_Clear():
        api.Function_Clear()

    @staticmethod
    def Function_GetList():
        """:return list({'mod': str, 'rvaStart': long, 'rvaEnd': long, 'manual': bool, 'instructioncount': long}) or None"""
        info = api.ListInfo()
        if not api.Function_GetList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.FunctionInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'mod': str(item.mod).replace('\0', ''),
                    'rvaStart': long(item.rvaStart),
                    'rvaEnd': long(item.rvaEnd),
                    'manual': item.manual,
                    'instructioncount': long(item.instructioncount)
                })

        finally:
            api.BridgeFree(info.data)

        return rv


class Label(object):
    @staticmethod
    def Set(*args):
        """
        :param duint addr:
        :param char const * text:
        :param bool manual=False:
        :return bool"""
        return api.Label_Set(*args)
        
    @staticmethod
    def SetByLabelInfo(*args):
        """
        :param LabelInfo info:
        :return bool"""
        return api.Label_SetByLabelInfo(*args)
    
    @staticmethod
    def FromString(label):
        """
        :param char const* label:
        :return duint addr or None"""
        rv = api.new_duintp()
        try:
            if api.Label_FromString(label, rv):
                return api.duintp_value(rv)
        finally:
            api.delete_duintp(rv)

    @staticmethod
    def Get(addr):
        """
        :param duint addr:
        :return str text or None"""
        rv = C.create_string_buffer(api.MAX_LABEL_SIZE)
        if api.Label_Get(addr, rv):
            return rv.value
        
    @staticmethod
    def GetInfo(addr):
        """
        :param duint addr:
        :return LabelInfo() or None"""
        rv = api.LabelInfo()
        if api.Label_GetInfo(addr, rv):
            return rv
    
    @staticmethod
    def Delete(addr):
        """
        :param duint addr:
        :return bool"""
        return api.Label_Delete(addr)
    
    @staticmethod
    def DeleteRange(start, end):
        """
        :param duint start:
        :param duint end"""
        api.Label_DeleteRange(start, end)
    
    @staticmethod
    def Clear():
        api.Label_Clear()
    
    @staticmethod
    def GetList():
        """:return list({'mod': str, 'rva': long, 'text': str, 'manual': bool}) or None"""
        info = api.ListInfo()
        if not api.Label_GetList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.LabelInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'mod': str(item.mod).replace('\0', ''),
                    'rva': long(item.rva),
                    'text': str(item.text).replace('\0', ''),
                    'manual': item.manual,
                })

        finally:
            api.BridgeFree(info.data)

        return rv


class Module(object):
    @staticmethod
    def InfoFromAddr(addr):
        """
        :param duint addr:
        :return ModuleInfo() or None"""
        return api.Module_InfoFromAddr(addr)

    @staticmethod
    def InfoFromName(name):
        """
        :param char const * name:
        :return ModuleInfo() or None"""
        mi = api.ModuleInfo()
        if api.Module_InfoFromName(name, mi):
            return mi

    @staticmethod
    def BaseFromAddr(addr):
        """
        :param duint addr:
        :returnduint"""
        return api.Module_BaseFromAddr(addr)

    @staticmethod
    def BaseFromName(name):
        """
        :param char const * name:
        :return duint"""
        return api.Module_BaseFromName(name)

    @staticmethod
    def SizeFromAddr(addr):
        """
        :param duint addr:
        :return duint"""
        return api.Module_SizeFromAddr(addr)

    @staticmethod
    def SizeFromName(name):
        """
        :param char const * name:
        :return duint"""
        return api.Module_SizeFromName(name)

    @staticmethod
    def NameFromAddr(addr):
        """
        :param duint addr:
        :return str name or None"""
        rv = C.create_string_buffer(api.MAX_MODULE_SIZE)
        if api.Module_NameFromAddr(addr, rv):
            return rv.value

    @staticmethod
    def PathFromAddr(addr):
        """
        :param duint addr:
        :return str path or None"""
        global __MAX_PATH
        rv = C.create_string_buffer(__MAX_PATH)
        if api.Module_PathFromAddr(addr, rv):
            return rv.value

    @staticmethod
    def PathFromName(name):
        """
        :param char const * name:
        :return str path or None"""
        global __MAX_PATH
        rv = C.create_string_buffer(__MAX_PATH)
        if api.Module_PathFromName(name, rv):
            return rv.value

    @staticmethod
    def EntryFromAddr(addr):
        """
        :param duint addr:
        :return duint"""
        return api.Module_EntryFromAddr(addr)

    @staticmethod
    def EntryFromName(name):
        """
        :param char const * name:
        :return duint"""
        return api.Module_EntryFromName(name)

    @staticmethod
    def SectionCountFromAddr(addr):
        """
        :param duint addr:
        :return int"""
        return api.Module_SectionCountFromAddr(addr)

    @staticmethod
    def SectionCountFromName(name):
        """
        :param char const * name:
        :return int"""
        return api.Module_SectionCountFromName(name)

    @staticmethod
    def SectionFromAddr(addr, number):
        """
        :param duint addr:
        :param int number:
        :return ModuleSectionInfo() or None"""

        rv = api.ModuleSectionInfo()
        if api.Module_SectionFromAddr(addr, number, rv):
            return rv

    @staticmethod
    def SectionFromName(name, number):
        """
        :param char const * name:
        :param int number:
        :return ModuleSectionInfo() or None"""
        rv = api.ModuleSectionInfo()
        if api.Module_SectionFromName(name, number, rv):
            return rv

    @staticmethod
    def SectionListFromAddr(addr):
        """
        :param duint addr:
        :return list({'addr': long, 'size': long, 'name': str}) or None"""
        info = api.ListInfo()
        if not api.Module_SectionListFromAddr(addr, info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.ModuleSectionInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'addr': long(item.addr),
                    'size': long(item.size),
                    'name': str(item.name).replace('\0', ''),
                })

        finally:
            api.BridgeFree(info.data)

        return rv

    @staticmethod
    def SectionListFromName(name):
        """
        :param char const * name:
        :return list({'addr': long, 'size': long, 'name': str}) or None"""

        info = api.ListInfo()
        if not api.Module_SectionListFromName(name, info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.ModuleSectionInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'addr': long(item.addr),
                    'size': long(item.size),
                    'name': str(item.name).replace('\0', ''),
                })

        finally:
            api.BridgeFree(info.data)

        return rv

    @staticmethod
    def GetMainModuleInfo():
        """
        :return ModuleInfo() or None"""
        rv = api.ModuleInfo()
        if api.Module_GetMainModuleInfo(rv):
            return rv

    @staticmethod
    def GetMainModuleBase():
        """
        :return duint"""
        return api.Module_GetMainModuleBase()

    @staticmethod
    def GetMainModuleSize():
        """
        :return duint"""
        return api.Module_GetMainModuleSize()

    @staticmethod
    def GetMainModuleEntry():
        """
        :return duint"""
        return api.Module_GetMainModuleEntry()

    @staticmethod
    def GetMainModuleSectionCount():
        """
        :return int"""
        return api.Module_GetMainModuleSectionCount()

    @staticmethod
    def GetMainModuleName():
        """
        :return str name or None"""
        rv = C.create_string_buffer(api.MAX_MODULE_SIZE)
        if api.Module_GetMainModuleName(rv):
            return rv.value

    @staticmethod
    def GetMainModulePath():
        """
        :return str path or None"""
        global __MAX_PATH
        rv = C.create_string_buffer(__MAX_PATH)
        if api.Module_GetMainModulePath(rv):
            return rv.value

    @staticmethod
    def GetMainModuleSectionList():
        """
        :return list({'addr': long, 'size': long, 'name': str}) or None"""

        info = api.ListInfo()
        if not api.Module_GetMainModuleSectionList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.ModuleSectionInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'addr': long(item.addr),
                    'size': long(item.size),
                    'name': str(item.name).replace('\0', ''),
                })

        finally:
            api.BridgeFree(info.data)

        return rv

    @staticmethod
    def GetList():
        """
        :return list({
            'base': long,
            'size': long,
            'entry': long,
            'sectionCount': int,
            'name': str,
            'path': str}) or None"""

        info = api.ListInfo()
        if not api.Module_GetList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.ModuleInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'base': long(item.base),
                    'size': long(item.size),
                    'entry': long(item.entry),
                    'sectionCount': int(item.sectionCount),
                    'name': str(item.name).replace('\0', ''),
                    'path': str(item.path).replace('\0', '')
                })

        finally:
            api.BridgeFree(info.data)

        return rv


class Symbol(object):
    @staticmethod
    def Symbol_GetList():
        """:return list({'mod': str, 'rva': long, 'name': str, 'manual': bool, 'type': Script::Symbol::SymbolType}) or None"""
        info = api.ListInfo()
        if not api.Symbol_GetList(info):
            return

        rv = list()
        try:
            count = info.count
            # size_bytes = info.size
            arr = api.SymbolInfoArray_frompointer(info.data)
            for _ in xrange(count):
                item = arr[_]
                rv.append({
                    'mod': str(item.mod).replace('\0', ''),
                    'rva': long(item.rva),
                    'name': str(item.name).replace('\0', ''),
                    'manual': item.manual,
                    'type': item.type
                })

        finally:
            api.BridgeFree(info.data)

        return rv
