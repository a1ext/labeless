/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include <QObject>

class JediCompletionWorker : public QObject
{
	Q_OBJECT
public:
	explicit JediCompletionWorker(QObject* parent = nullptr);
	~JediCompletionWorker();

signals:
	void completeError(const QString& error);
	void completeFinished();

public slots:
	void main();
};