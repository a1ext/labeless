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


#define __LOG_PREFIX L"LL: "
#ifdef _UNICODE
#	define __XTT(str) L ## str
#else
#	define __XTT(str) str
#endif // _UNICODE
#define _STR_TT(str) __XTT(str)
#define __WFUNCTION__ _STR_TT(__FUNCTION__)

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > xstring;

// fwd
namespace util {
	xstring xformat(const wchar_t* fmt, ...);
} // util


#define __LOG_IMPL(FUN, TYPE, FMT, ...) do {						\
	xstring _s = util::xformat(__LOG_PREFIX FUN FMT, __VA_ARGS__);	\
	if (_s.size() >= TEXTLEN)										\
		_s.erase(TEXTLEN - 1, _s.length() - TEXTLEN + 1);			\
	Addtolist(0, (TYPE), L"%s", _s.c_str());						\
} while (0)

#define _CAT(X, Y) X##Y

#define log_r(FMT, ...)			__LOG_IMPL(_CAT(_STR_TT(__FUNCTION__), _STR_TT(": ")), 1, _STR_TT(FMT), __VA_ARGS__)
#define log_r_no_fn(FMT, ...)	__LOG_IMPL(_STR_TT(""), 1, _STR_TT(FMT), __VA_ARGS__)

#define log_g(FMT, ...)			__LOG_IMPL(_CAT(_STR_TT(__FUNCTION__), _STR_TT(": ")), -1, _STR_TT(FMT), __VA_ARGS__)
#define log_g_no_fn(FMT, ...)	__LOG_IMPL(_STR_TT(""), -1, _STR_TT(FMT), __VA_ARGS__)

