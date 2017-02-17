/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "util_protobuf.h"

// protobuf
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>

namespace util {
namespace protobuf {

bool parseBigMessage(::google::protobuf::Message& msg, const std::string& data)
{
	static const int kProtobufMessageLimit = 0x40000000;

	::google::protobuf::io::CodedInputStream input(reinterpret_cast<const ::google::protobuf::uint8*>(data.c_str()), data.size());
	input.SetTotalBytesLimit(kProtobufMessageLimit, kProtobufMessageLimit);
	return msg.ParseFromCodedStream(&input);
}

} // protobuf
} // util
