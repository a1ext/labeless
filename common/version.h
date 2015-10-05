/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once


#define _MAKE_VER(A, B, C, D) ((unsigned((A) & 0xFF) << 24) | (unsigned((B) & 0xFF) << 16) | (unsigned((C) & 0xFF) << 8) | ((D) & 0xFF))
#define _STR(X) #X
#define STR(X) _STR(X)



#define VERSION_MAJOR               1
#define VERSION_MINOR               0
#define VERSION_REVISION            0
#define VERSION_BUILD               7


#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD

#define LABELESS_VER_INT            _MAKE_VER(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD)
#define LABELESS_VER_STR            STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_REVISION) "." STR(VERSION_BUILD)

#define LABELESS_PRODUCT            "Labeless"
