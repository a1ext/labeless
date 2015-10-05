/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <QObject>

class RpcThreadWorker : public QObject
{
	Q_OBJECT
public:
	explicit RpcThreadWorker(QObject* parent = nullptr);
	~RpcThreadWorker();

public slots:
	void main();
};

