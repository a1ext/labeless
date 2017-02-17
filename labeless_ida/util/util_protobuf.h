/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include <string>

// fwd
namespace google	{
namespace protobuf	{

class Message;

} // protobuf
} // google

namespace util {
namespace protobuf {

bool parseBigMessage(::google::protobuf::Message& msg, const std::string& data);

} // protobuf
} // util