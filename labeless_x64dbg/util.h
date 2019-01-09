/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <deque>
#include <string>

#include "types.h"

struct sockaddr_in;

namespace util {

xstring getHostAppDir();
xstring getPluginDir(HINSTANCE hPlugin);
xstring inetAddrToString(sockaddr_in* sin);

std::wstring mb2w(const std::string& v);
std::string w2mb(const std::wstring& v);

std::deque<std::string> split(const std::string& s, const std::string& delimitersRE = "\r|\n");
std::string randStr(int len);

inline xstring to_xstr(const char* v)
{
	if (!v)
		return {};
	return xstring(v);
}

inline xstring to_xstr(const wchar_t* v)
{
	if (!v)
		return {};
	return w2mb(v);
}

std::deque<std::string> getNetworkInterfacess();
bool copyToClipboard(HWND h, const std::string& data);

/*#ifdef _UNICODE
#	define to_xstr(X) mb2w((X))
#else // _UNICODE
#	define to_xstr(X) xstring((X))
#endif // _UNICODE*/

} // util

