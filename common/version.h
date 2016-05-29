/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#define _MAKE_VER(A, B, C, D) ((unsigned((A) & 0xFF) << 24) | (unsigned((B) & 0xFF) << 16) | (unsigned((C) & 0xFF) << 8) | ((D) & 0xFF))
#ifdef _UNICODE
#	define __TTT(X) L ## X
#	define _SSTR(X) __TTT(#X)
#else // _UNICODE
#	define __TTT(X) X
#	define _SSTR(X) #X
#endif // _UNICODE

#define SSTR(X) _SSTR(X)


#define VERSION_MAJOR               1
#define VERSION_MINOR               1
#define VERSION_REVISION            0
#define VERSION_BUILD               4


#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD

#define LABELESS_VER_INT            _MAKE_VER(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD)
#define LABELESS_VER_STR            SSTR(VERSION_MAJOR) __TTT(".") SSTR(VERSION_MINOR) __TTT(".") SSTR(VERSION_REVISION) __TTT(".") SSTR(VERSION_BUILD)

#define LABELESS_PRODUCT            "Labeless"
