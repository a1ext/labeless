/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// kludge to avoid link with debug version of python27
#ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
#else
#  include <Python.h>
#endif // _DEBUG

#include <Shlwapi.h>
#include "sdk/Plugin.h"

#include <string>


#define __LOG_PREFIX "LL: "

#define __LOG_IMPL(FUN, TYPE, FMT, ...) do {						\
	Addtolist(0, (TYPE), __LOG_PREFIX FUN FMT, __VA_ARGS__);		\
} while (0)

#define _CAT(X, Y) X##Y

#define log_r(FMT, ...)			__LOG_IMPL(_CAT(__FUNCTION__,": "), 1, FMT, __VA_ARGS__)
#define log_r_no_fn(FMT, ...)	__LOG_IMPL("", 1, FMT, __VA_ARGS__)

#define log_g(FMT, ...)			__LOG_IMPL(_CAT(__FUNCTION__,": "), -1, (FMT), __VA_ARGS__)
#define log_g_no_fn(FMT, ...)	__LOG_IMPL("", -1, FMT, __VA_ARGS__)
