#include "pausenotificationlistener.h"

#include <memory>
#include <QApplication>

#include "labeless_ida.h"
#include "util/util_protobuf.h"
#include "../common/cpp/rpc.pb.h"

PauseNotificationListener::PauseNotificationListener(QObject* parent)
	: QObject(parent)
{
	msg("%s\n", Q_FUNC_INFO);
}

PauseNotificationListener::~PauseNotificationListener()
{
	msg("%s\n", Q_FUNC_INFO);
}

bool PauseNotificationListener::handlePacket(const std::string& packet)
{
	auto response = new rpc::PausedNotification();
	const bool parsedOk = util::protobuf::parseBigMessage(*response, packet);
	if (parsedOk)
	{
		// TODO: verify, transform
		emit received(response);
	}
	return parsedOk;
}

void PauseNotificationListener::main()
{
	int bTrue = 1;
	SOCKADDR_IN sinBCast = {};
	sinBCast.sin_family = AF_INET;
	sinBCast.sin_port = ntohs(Labeless::instance().pauseNotificationPort());
	sinBCast.sin_addr.S_un.S_addr = INADDR_ANY;

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (INVALID_SOCKET == s ||
		SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&bTrue), sizeof(bTrue)) ||
		SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&bTrue), sizeof(bTrue)) ||
		SOCKET_ERROR == bind(s, reinterpret_cast<const sockaddr*>(&sinBCast), sizeof(sinBCast)))
	{
		msg("%s: cannot create UDP socket, le: %x\n", Q_FUNC_INFO, GetLastError());
		moveToThread(qApp->thread());
		deleteLater();
		return;
	}
	std::shared_ptr<void> guard(nullptr, [s](void*) {
#ifdef __NT__
		closesocket(s);
#else
		close(s);
#endif
	}); // clean-up guard
	Q_UNUSED(guard);

	timeval timeout = { 1, 0 };
	fd_set readSet;
	char buff[1024] = {};
	SOCKADDR_IN sin;
	int sinLen;

	Labeless& ll = Labeless::instance();
	while (ll.m_Enabled == 1 && ll.pauseNotificationHanlingEnabled())
	{
		FD_ZERO(&readSet);
		FD_SET(s, &readSet);

		if (select(1, &readSet, nullptr, nullptr, &timeout) >= 0 &&
			FD_ISSET(s, &readSet))
		{
			sinLen = static_cast<int>(sizeof(sin));
			const int cnt = recvfrom(s, buff, sizeof(buff), 0, reinterpret_cast<sockaddr*>(&sin), &sinLen);
			if (cnt >= 0 && cnt < sizeof(buff))
			{
				if (!handlePacket(std::string(buff, static_cast<size_t>(cnt))))
				{
					msg("%s: handlePacket() failed\n"); // remove from release
				}
			}
		}
	}

	moveToThread(qApp->thread());
	deleteLater();
}
