/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "util.h"

#include <WinSock2.h>
#include <cstdlib>
#include <ShlObj.h>
#include <regex>
#include <vector>

namespace util {

std::string getOllyDir()
{
	char buff[MAX_PATH] = {};
	GetModuleFileName(HINSTANCE(Plugingetvalue(VAL_HINST)), buff, MAX_PATH);
	PathRemoveFileSpec(buff);
	return std::string(buff);
}

std::string getPluginDir(HINSTANCE hPlugin)
{
	char buff[MAX_PATH] = {};
	GetModuleFileName(hPlugin, buff, MAX_PATH);
	PathRemoveFileSpec(buff);
	return std::string(buff);
}

std::string getErrorDir()
{
	std::string rv(MAX_PATH, '\0');
	if (S_OK != SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, &rv[0]))
		return {};
	rv.resize(strnlen_s(rv.c_str(), MAX_PATH));
	rv += "\\.labeless\\errors";
	SHCreateDirectoryEx(NULL, rv.c_str(), NULL);
	return rv;
}

std::string inetAddrToString(sockaddr_in* sin)
{
	char buff[MAX_PATH] = {};
	DWORD buffLen = MAX_PATH;
	if (SOCKET_ERROR == WSAAddressToString(reinterpret_cast<sockaddr*>(sin), sizeof(sockaddr_in), nullptr, buff, &buffLen))
	{
		return std::string();
	}
	return std::string(buff);
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

std::deque<std::string> split(const std::string& s, const std::string& delimitersRE)
{
	std::deque<std::string> rv;

	const std::string fixed = std::regex_replace(s, std::regex("\r"), "");

	std::regex rePattern(delimitersRE);
	std::sregex_token_iterator iter(fixed.begin(), fixed.end(), rePattern, -1);
	std::sregex_token_iterator end;
	for (; iter != end; ++iter)
	{
		const std::string& item = *iter;
		if (!item.empty())
			rv.push_back(item);
	}
	return rv;
}

} // util