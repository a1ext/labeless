/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "util.h"
#include <vector>
#include <WinSock2.h>

namespace util {

xstring getOllyDir()
{
	return ollydir;
}

xstring getPluginDir(HINSTANCE hPlugin)
{
	TCHAR buff[MAX_PATH] = {};
	GetModuleFileName(hPlugin, buff, MAX_PATH);
	PathRemoveFileSpec(buff);
	return std::wstring(buff);
}

xstring inetAddrToString(sockaddr_in* sin)
{
	TCHAR buff[MAX_PATH] = {};
	DWORD buffLen = MAX_PATH;
	if (SOCKET_ERROR == WSAAddressToString(reinterpret_cast<sockaddr*>(sin), sizeof(sockaddr_in), nullptr, buff, &buffLen))
	{
		return xstring();
	}
	return xstring(buff);
}

std::string sformat(const char* fmt, ...)
{
	static const size_t kBuffSize = 4096 * 4096;
	std::vector<char> buff(kBuffSize, '\0');
	va_list v;
	va_start(v, fmt);
	_vsnprintf_s(&buff[0], kBuffSize, _TRUNCATE, fmt, v);
	va_end(v);
	return std::string(&buff[0]);
}

std::wstring mb2w(const std::string& v)
{
	if (v.empty())
		return{};

	size_t len = v.length();

	int iLen = MultiByteToWideChar(CP_ACP, 0, v.c_str(), len, nullptr, 0);
	if (iLen <= 0)
		return{};
	std::wstring rv(iLen, L'\0');
	iLen = MultiByteToWideChar(CP_ACP, 0, v.c_str(), len, &rv[0], iLen);
	if (iLen > 0)
		return rv;
	return{};
}

std::string w2mb(const std::wstring& v)
{
	if (v.empty())
		return{};

	size_t wideLen = v.length();

	int iLen = WideCharToMultiByte(CP_ACP, 0, v.c_str(), wideLen, nullptr, 0, NULL, NULL);
	if (iLen <= 0)
		return{};

	std::string rv(iLen, '\0');
	iLen = WideCharToMultiByte(CP_ACP, 0, v.c_str(), wideLen, &rv[0], iLen, NULL, NULL);
	if (iLen)
		return rv;
	return{};
}


} // util