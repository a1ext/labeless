/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <string>
#include "types.h"

namespace google	{
namespace protobuf	{

class Message;

} // protobuf
} // google

namespace hlp
{

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);

std::string unhexlify(const std::string& hexStr);
void addLogMsg(const char* fmt, ...);
std::string memoryProtectToStr(quint32 p);
qlist<ea_t> codeRefsToCode(ea_t ea);
qlist<ea_t> dataRefsToCode(ea_t ea);
ea_t getNextCodeOrDataEA(ea_t ea, bool nonCodeNames);
bool isFuncStart(ea_t ea);

namespace protobuf {

bool parseBigMessage(::google::protobuf::Message& msg, const std::string& data);

} // protobuf

namespace net {


qstring wsaErrorToString();
bool sockSendBuff(SOCKET s, const char* buff, uint64_t len);
bool sockSendString(SOCKET s, const std::string& str);
bool sockRecvAll(SOCKET s, std::string& result);
bool sendAll(SOCKET s, const std::string& buff, std::string& error);

} // net
} // hlp

