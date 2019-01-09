/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "util.h"

#include <WinSock2.h>
#include <IPHlpApi.h>
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

std::deque<std::string> getNetworkInterfacess()
{
	std::deque<std::string> rv;

	ULONG infoLen = 0;
	ULONG erradapt = ::GetAdaptersInfo(nullptr, &infoLen);
	if (ERROR_BUFFER_OVERFLOW != erradapt)
		return rv;

	std::string buff(infoLen, '\0');
	erradapt = ::GetAdaptersInfo(PIP_ADAPTER_INFO(&buff[0]), &infoLen);
	if (ERROR_SUCCESS != erradapt)
		return rv;

	PIP_ADAPTER_INFO pInfo = PIP_ADAPTER_INFO(&buff[0]);

	do
	{
		IP_ADDR_STRING* pNext = &pInfo->IpAddressList;
		while (pNext)
		{
			rv.push_back(pNext->IpAddress.String);
			pNext = pNext->Next;
		}
		pInfo = pInfo->Next;
	} while (pInfo);

	return rv;
}

bool copyToClipboard(HWND h, const std::string& data)
{
	if (data.empty())
		return false;

	if (!OpenClipboard(h))
	{
		Addtolist(0, 1, "LL: Cannot open clipboard");
		return false;
	}

	EmptyClipboard();
	bool rv = false;
	if (HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, data.length() + 1))
	{
		char* pOut = reinterpret_cast<char*>(GlobalLock(hGlob));
		if (pOut)
		{
			strncpy_s(pOut, data.length() + 1, data.c_str(), data.length());
			SetClipboardData(CF_TEXT, pOut);
			GlobalUnlock(hGlob);
			rv = true;
		}
		else
		{
			Addtolist(0, 1, "LL: GlobalLock failed");
		}
	}
	else
	{
		Addtolist(0, 1, "LL: GlobalAlloc failed");
	}
	CloseClipboard();

	return rv;
}

} // util