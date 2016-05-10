import ctypes as C
from . import api


def DbgIsDebugging():
    return api.DbgIsDebugging()


def GuiGetLineWindow(title=''):
    line = C.create_string_buffer(api.GUI_MAX_LINE_SIZE)
    return_value = api.GuiGetLineWindow("%s" % title, line)
    if return_value:
        return line.value


def GuiGetWindowHandle():
    return api.GuiGetWindowHandle()


def BridgeSettingGet(section, key):
    """BridgeSettingGet(section, key) -> str or None"""
    value = C.create_string_buffer(api.MAX_SETTING_SIZE)
    if api.BridgeSettingGet(section, key, value):
        return value.value


def DbgGetLabelAt(addr, segment):
    """DbgGetLabelAt(duint addr, SEGMENTREG segment) -> str or None"""
    rv = C.create_string_buffer(api.MAX_LABEL_SIZE)
    if api.DbgGetLabelAt(addr, segment, rv):
        return rv.value


def DbgGetCommentAt(addr):
    """DbgGetCommentAt(duint addr) -> str or None"""
    rv = C.create_string_buffer(api.MAX_COMMENT_SIZE)
    if api.DbgGetCommentAt(addr, rv):
        return rv.value


def DbgGetModuleAt(addr):
    """DbgGetModuleAt(duint addr) -> str or None"""
    rv = C.create_string_buffer(api.MAX_MODULE_SIZE)
    if api.DbgGetModuleAt(addr, rv):
        return rv.value


def DbgGetStringAt(addr):
    """DbgGetStringAt(duint addr) -> str or None"""
    rv = C.create_string_buffer(api.MAX_MODULE_SIZE)
    if api.DbgGetStringAt(addr, rv):
        return rv.value


def GuiGetDisassembly(addr):
    """GuiGetDisassembly(duint addr) -> str or None"""
    rv = C.create_string_buffer(api.MAX_MODULE_SIZE)
    if api.GuiGetDisassembly(addr, rv):
        return rv.value

