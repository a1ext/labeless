# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons CC BY-NC-ND 3.0
# http://creativecommons.org/licenses/by-nc-nd/3.0/

__author__ = 'a1ex_t'

import _py_olly


def std_out_handler(*args):
    """ stdout handler
    :param args:
    :return:
    """
    return _py_olly.std_out_handler(*args)
std_out_handler = _py_olly.std_out_handler


def std_err_handler(*args):
    """ stderr handler
    :param args:
    :return:
    """
    return _py_olly.std_err_handler(*args)
std_err_handler = _py_olly.std_err_handler


def set_binary_result(*args):
    """ Set rpc binary result back to olly
    :param args:
    :return:
    """
    return _py_olly.set_binary_result(*args)
set_binary_result = _py_olly.set_binary_result


def get_params(*args):
    """
    :param args:
    :return: binary serialized Rpc Request
    """
    return _py_olly.get_params(*args)
get_params = _py_olly.get_params


def olly_log(*args):
    """ Log to Olly's log window
    :param : message
    """
    return _py_olly.olly_log(*args)
olly_log = _py_olly.olly_log


def set_error(*args):
    """ Set error message to Olly
    :param : message
    """
    return _py_olly.set_error(*args)
set_error = _py_olly.set_error


def labeless_ver():
	""" Gets Labeless version"""
	return _py_olly.labeless_ver()
labeless_ver = _py_olly.labeless_ver
