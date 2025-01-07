/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "util_protobuf.h"

// protobuf
#if defined(__NT__)
#	pragma warning(push)
#	pragma warning(disable:4800)
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // defined(__GNUC__)
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#if defined(__NT__)
#	pragma warning(pop)
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif // defined(__GNUC__)


namespace util {
namespace protobuf {

bool parseBigMessage(::google::protobuf::Message& msg, const std::string& data)
{
	static const int kProtobufMessageLimit = 0x40000000;

	::google::protobuf::io::CodedInputStream input(
		reinterpret_cast<const ::google::protobuf::uint8*>(data.c_str()),
		static_cast<int>(data.size())
	);
	input.SetTotalBytesLimit(kProtobufMessageLimit);
	return msg.ParseFromCodedStream(&input);
}

} // protobuf
} // util
