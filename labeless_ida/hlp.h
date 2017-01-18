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

#include <QString>

QT_FORWARD_DECLARE_CLASS(QMainWindow)

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
QMainWindow* findIDAMainWindow();

namespace idapython {

bool init();
bool runScript(const std::string& script, std::string& externObj, std::string& error);
bool setResultObject(const std::string& obj, std::string& error);

} // idapython


namespace python {

// direct Python bindings
bool init(QString& error);
bool safeRunString(const std::string& script, bool& exceptionOccured, std::string& error);

} // python

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

namespace github {

struct ReleaseInfo
{
	QString tag; // tag_name
	QString name; // name
	QString url; // html_url
};

bool getLatestRelease(ReleaseInfo& ri, std::string& error);

} // github

} // hlp

