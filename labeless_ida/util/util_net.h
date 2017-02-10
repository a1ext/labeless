/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include "../types.h"

namespace util {

namespace net {

QString wsaErrorToString();
bool sockSendBuff(SOCKET s, const char* buff, uint64_t len);
bool sockSendString(SOCKET s, const std::string& str);
bool sockRecvAll(SOCKET s, std::string& result);
bool sendAll(SOCKET s, const std::string& buff, std::string& error);

} // net

} // util
