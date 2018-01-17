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

bool broadcastPaused(const PausedInfo& info, const GUID& instanceId)
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (INVALID_SOCKET == s)
		return false;

	int bTrue = 1;
	if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&bTrue), sizeof(bTrue)))
	{
		closesocket(s);
		return false;
	}

	sockaddr_in sin = {};
	sin.sin_port = ntohs(12344); // TODO
	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = INADDR_BROADCAST;

	rpc::PausedNotification pn;
	pn.set_backend_id(&instanceId, sizeof(instanceId));

	auto info32 = pn.mutable_info32();
	info32->set_ip(info.ip);
	info32->set_flags(info.flags);

	for (unsigned i = 0; i < NREG; ++i)
		info32->add_r(info.r[i]);

	for (unsigned i = 0; i < NSEG; ++i)
		info32->add_s(info.s[i]);

	const std::string& serialized = pn.SerializeAsString();
	const bool ok = SOCKET_ERROR != sendto(s, reinterpret_cast<const char*>(serialized.c_str()), serialized.size(), 0, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin));
	closesocket(s);
	return ok;
}

} // util
