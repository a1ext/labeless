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
#include <WinSock2.h>

// kludge to avoid link with debug version of python27
#ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
#else
#  include <Python.h>
#endif // _DEBUG

#include <Shlwapi.h>
#include "pluginsdk/_plugins.h"

#include <string>


#define __LOG_PREFIX "LL: "


#ifdef _UNICODE
#	define __XTT(str) L ## str
#else
#	define __XTT(str) str
#endif // _UNICODE
#define _STR_TT(str) __XTT(str)
#define __XFUNCTION__ _STR_TT(__FUNCTION__)

#define __LOG_IMPL(FUN, TYPE, FMT, ...) do {					\
	_plugin_logprintf(__LOG_PREFIX FUN FMT, __VA_ARGS__);		\
} while (0)

#define _CAT(X, Y) X##Y

#define log_r(FMT, ...)			__LOG_IMPL(_CAT(__FUNCTION__,": "), 1, _CAT(FMT, "\n"), __VA_ARGS__)
#define log_r_no_fn(FMT, ...)	__LOG_IMPL("", 1, _CAT(FMT, "\n"), __VA_ARGS__)

#define log_g(FMT, ...)			__LOG_IMPL(_CAT(__FUNCTION__,": "), -1, _CAT(FMT, "\n"), __VA_ARGS__)
#define log_g_no_fn(FMT, ...)	__LOG_IMPL("", -1, _CAT(FMT, "\n"), __VA_ARGS__)

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > xstring;
typedef std::basic_stringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > xstringstream;

#if _WIN64 || __amd64__
// #error "x86_64 build is not supported for now"
#endif // _WIN64 || __amd64__

#ifndef PRIXPTR
#	ifdef _WIN64
#		define _PFX_PTR  "ll"
#	else
#		define _PFX_PTR  "l"
#	endif
#	define PRIXPTR      _PFX_PTR "X"
#endif // PRIXPTR
