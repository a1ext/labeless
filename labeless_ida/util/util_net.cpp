/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "util_net.h"
#include "util_ida.h"

#if defined(__unix__) || defined(__linux__)
#include <string.h>
#endif

#include <limits>
#include <sstream>

#undef max // fix for std::numeric_limits<>::max()

namespace util {

namespace net {

QString wsaErrorToString()
{
	QString rv;
#ifdef __NT__
	const DWORD e = WSAGetLastError();
	rv = QString("\n0x%1 ").arg(e, 8, 16, QChar('0'));

	const char* msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, e,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&msg, 0, nullptr);
	if (!msg)
		return rv;
	rv += QString(msg);
	LocalFree((HLOCAL)msg);
#elif defined(__unix__) || defined(__linux__)
	if (auto errStr = strerror(errno))
		rv = errStr;
#endif
	return rv;
}

bool sockSendBuff(SOCKET s, const char* buff, uint64_t len)
{
	if (INVALID_SOCKET == s || !buff || !len)
		return false;

	static const int kMaxInt = std::numeric_limits<int>::max();
	uint64_t left = len;

	while (left > 0)
	{
		const int toSend = left > kMaxInt ? left % kMaxInt : static_cast<int>(left);
		const int retVal = send(s, buff, toSend, 0);
		if (retVal < 0)
		{
			ida::addLogMsg(QString("send() failed. Error: %1\n").arg(wsaErrorToString()).toStdString().c_str());
			return false;
		}
		left -= retVal;
		buff += retVal;
	}

	return true;
}

bool sockSendString(SOCKET s, const std::string& str)
{
	if (INVALID_SOCKET == s || str.empty())
		return false;

	const char* ptrBuffer = str.c_str();
	static const int kMaxInt = std::numeric_limits<int>::max();

	uint64_t left = static_cast<uint64_t>(str.length());

	while (left > 0)
	{
		const int toSend = left > kMaxInt ? left % kMaxInt : static_cast<int>(left);

		const int retVal = send(s, ptrBuffer, toSend, 0);
		if (retVal < 0)
		{
			ida::addLogMsg(QString("send() failed. Error: %1\n").arg(wsaErrorToString()).toStdString().c_str());
			return false;
		}
		left -= retVal;
		ptrBuffer += retVal;
	}
	return true;
}

bool sockRecvAll(SOCKET s, std::string& result)
{
	result.clear();

	static const uint kBuffSize = 0x5000;
	char buff[kBuffSize];
	int rv = 0;
	std::stringstream response;
	while ((rv = recv(s, buff, kBuffSize, 0)) > 0)
		response << std::string(buff, buff + rv);
	result = response.str();
	return true;
}

bool sendAll(SOCKET s, const std::string& buff, std::string& error)
{
	unsigned total_sent = 0;
	int sent;
	while (total_sent < buff.length())
	{
		if (SOCKET_ERROR == (sent = send(s, buff.c_str() + total_sent, buff.length() - total_sent, 0)))
		{
			error = wsaErrorToString().toStdString();
			return false;
		}
		total_sent += static_cast<unsigned>(sent);
	}
	return total_sent == buff.size();
}

} // net

} // util