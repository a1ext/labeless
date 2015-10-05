/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "hlp.h"
#include "labeless_ida.h"
#include <WinSock2.h>

#include <sstream>
#include <QString>

#include <QMetaObject>

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>


namespace hlp
{

static const std::string base64_chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	std::stringstream ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret << base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret << base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret << '=';

	}

	return ret.str();
}

std::string base64_decode(const std::string& encoded_string)
{
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::stringstream ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret << char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret << char_array_3[j];
	}

	return ret.str();
}

std::string unhexlify(const std::string& hexStr)
{
	const size_t cnt = hexStr.size() / 2;
	std::string rv(cnt, '\0');
	for (size_t i = 0; i < cnt; ++i)
		rv[i] = char(std::stoul(hexStr.substr(i * 2, 2), nullptr, 16));
	return rv;
}

void addLogMsg(const char* fmt, ...)
{
	static const QString kRPCThread = "RPC Thread";
	QString message;
	do {
		va_list va;
		va_start(va, fmt);
		char buff[1024] = {};
		const int len = ::qvsnprintf(buff, sizeof(buff), fmt, va);
		va_end(va);
		message = buff;
	} while (0);

	QMetaObject::invokeMethod(&Labeless::instance(), "onLogMessage", Qt::QueuedConnection,
		Q_ARG(QString, message),
		Q_ARG(QString, kRPCThread));
}

std::string memoryProtectToStr(DWORD p)
{
	std::string rv;

	if ((p & PAGE_READONLY) ||
		(p & PAGE_READWRITE) ||
		(p & PAGE_EXECUTE_READ) ||
		(p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY) ||
		(p & SEC_IMAGE))
		rv += "R";
	else
		rv += " ";

	if ((p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY) ||
		(p & PAGE_READWRITE) ||
		(p & PAGE_WRITECOMBINE))
		rv += "W";
	else
		rv += " ";

	if ((p & PAGE_EXECUTE) ||
		(p & PAGE_EXECUTE_READ) ||
		(p & PAGE_EXECUTE_READWRITE) ||
		(p & PAGE_EXECUTE_WRITECOPY))
		rv += "E";
	else
		rv += " ";

	if (p & PAGE_NOACCESS)
		rv = "NO ACCESS";

	if (p & PAGE_GUARD)
		rv += " GUARD";
	return rv;
}

qlist<ea_t> codeRefsToCode(ea_t ea)
{
	if (!isCode(get_flags_novalue(ea)))
		return qlist<ea_t>();
	ea_t eCref = get_first_fcref_to(ea);
	if (eCref == BADADDR)
		return qlist<ea_t>();

	qlist<ea_t> refs;
	do {
		refs.push_back(eCref);
	} while ((eCref = get_next_fcref_to(ea, eCref)) != BADADDR);
	return refs;
}

qlist<ea_t> dataRefsToCode(ea_t ea)
{
	if (!isCode(get_flags_novalue(ea)))
		return qlist<ea_t>();
	ea_t eDref = get_first_dref_to(ea);
	if (eDref == BADADDR)
		return qlist<ea_t>();

	qlist<ea_t> refs;
	do {
		refs.push_back(eDref);
	} while ((eDref = get_next_dref_to(ea, eDref)) != BADADDR);
	return refs;
}

ea_t getNextCodeOrDataEA(ea_t ea, bool nonCodeNames)
{
	while (BADADDR != (ea = next_not_tail(ea)))
	{
		auto flags = get_aflags(ea);
		if ((flags & (1 << 0xE)))
			return ea;
		flags = get_flags_ex(ea, 1);
		if (::isEnabled(ea) && (isCode(flags) || (nonCodeNames && isData(flags))))
			return ea;
	}
	return BADADDR;
}

namespace protobuf {

bool parseBigMessage(::google::protobuf::Message& msg, const std::string& data)
{
	static const int kProtobufMessageLimit = 0x40000000;

	::google::protobuf::io::CodedInputStream input(reinterpret_cast<const uint8*>(data.c_str()), data.size());
	input.SetTotalBytesLimit(kProtobufMessageLimit, kProtobufMessageLimit);
	return msg.ParseFromCodedStream(&input);
}

} // protobuf

namespace net {

qstring wsaErrorToString()
{
	qstring rv;
	const DWORD e = WSAGetLastError();
	rv.sprnt("\n0x%08X ", e);

	char* msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, e,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&msg, 0, nullptr);
	if (!msg)
		return rv;
	rv += msg;
	LocalFree((HLOCAL)msg);
	return rv;
}

bool sockSendString(SOCKET s, const std::string& str)
{
	if (INVALID_SOCKET == s || str.empty())
		return false;

	const char* ptrBuffer = str.c_str();
	int bytesToSend = int(str.length());

	while (bytesToSend > 0)
	{
		const int retVal = send(s, ptrBuffer, bytesToSend, 0);
		if (SOCKET_ERROR == retVal)
		{
			addLogMsg(QString("send() failed. Error: %1\n").arg(wsaErrorToString().c_str()).toStdString().c_str());
			return false;
		}
		bytesToSend -= retVal;
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
			error = wsaErrorToString().c_str();
			return false;
		}
		total_sent += static_cast<unsigned>(sent);
	}
	return total_sent == buff.size();
}

} // net
} // hlp

