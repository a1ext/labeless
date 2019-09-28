/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "util.h"
#include <iomanip>
#include <regex>
#include <sstream>
#include <vector>
#include <WinSock2.h>
#include <IPHlpApi.h>
#include "../common/cpp/rpc.pb.h"

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

xstring xformat(const wchar_t* fmt, ...)
{
	static const size_t kBuffSize = 4096 * 4096;
	std::vector<wchar_t> buff(kBuffSize, '\0');
	va_list v;
	va_start(v, fmt);
	_vsnwprintf_s(&buff[0], kBuffSize, _TRUNCATE, fmt, v);
	va_end(v);
	return xstring(&buff[0]);
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

std::string hexlify(const BYTE* data, SIZE_T len)
{
	std::stringstream rv;
	rv << std::hex;
	for (SIZE_T i = 0; i < len; ++i)
		rv << std::setw(2) << std::setfill('0') << unsigned(data[i]);

	return rv.str();
}

std::deque<xstring> split(const xstring& s, const xstring& delimitersRE)
{
	std::deque<xstring> rv;

	const xstring fixed = std::regex_replace(s, std::wregex(L"\r"), L"");

	std::wregex rePattern(delimitersRE);
	std::wsregex_token_iterator iter(fixed.begin(), fixed.end(), rePattern, -1);
	std::wsregex_token_iterator end;
	for (; iter != end; ++iter)
	{
		const xstring& item = *iter;
		if (!item.empty())
			rv.push_back(item);
	}
	return rv;
}

std::string randStr(int len)
{
	srand(GetTickCount());

	auto randChar = []() {
		char c = '\0';
		do {
			c = rand() % 0xFF;
		} while (!::isalpha(c));
		return c;
	};

	std::string rv(len, '\0');
	for (int i = 0; i < len; ++i)
		rv[i] = randChar();

	return rv;
}

std::deque<xstring> getNetworkInterfacess()
{
	std::deque<xstring> rv;

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
			rv.push_back(mb2w(pNext->IpAddress.String));
			pNext = pNext->Next;
		}
		pInfo = pInfo->Next;
	} while (pInfo);

	return rv;
}

bool copyToClipboard(HWND h, const xstring& data)
{
	if (data.empty())
		return false;

	if (!OpenClipboard(h))
	{
		Addtolist(0, RED, L"LL: Cannot open clipboard");
		return false;
	}

	EmptyClipboard();
	bool rv = false;
	if (HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, (data.length() + 1) * sizeof(wchar_t)))
	{
		wchar_t* pOut = reinterpret_cast<wchar_t*>(GlobalLock(hGlob));
		if (pOut)
		{
			wcsncpy_s(pOut, data.length() + 1, data.c_str(), data.length());
			SetClipboardData(CF_UNICODETEXT, pOut);
			GlobalUnlock(hGlob);
			rv = true;
		}
		else
		{
			Addtolist(0, RED, L"LL: GlobalLock failed");
		}
	}
	else
	{
		Addtolist(0, RED, L"LL: GlobalAlloc failed");
	}
	CloseClipboard();

	return rv;
}

} // util
