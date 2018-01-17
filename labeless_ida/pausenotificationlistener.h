#pragma once

#include <QObject>
#include "types.h"


class PauseNotificationListener : public QObject
{
	Q_OBJECT
public:
	explicit PauseNotificationListener(QObject* parent = nullptr);
	~PauseNotificationListener();

private:
	bool handlePacket(const std::string& packet);

public slots:
	void main();

signals:
	void received(void*); // TODO
};

