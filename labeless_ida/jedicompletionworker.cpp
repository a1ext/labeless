/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "jedicompletionworker.h"
#include "labeless_ida.h"
#include "util/util_python.h"
#include "util/util_ida.h"
#include "jedi.h"

#include <QApplication>


JediCompletionWorker::JediCompletionWorker(QObject* parent)
	: QObject(parent)
{
	msg("%s\n", Q_FUNC_INFO);
}

JediCompletionWorker::~JediCompletionWorker()
{
	msg("%s\n", Q_FUNC_INFO);
}

void JediCompletionWorker::main()
{
	static const unsigned long waitTime = 2000;

	std::shared_ptr<void> threadGuard(nullptr, [this](void*) {
		moveToThread(qApp->thread());
		deleteLater();
	}); // clean-up guard
	Q_UNUSED(threadGuard);

	QString error;
	if (!util::python::init(error))
	{
		util::ida::addLogMsg("util::python::init() failed, error: %s\n", error.toStdString().c_str());
		return;
	}

	if (!util::python::jedi::init(error))
	{
		util::ida::addLogMsg("util::python::jedi::init() failed, error: %s\n", error.toStdString().c_str());
		return;
	}

	util::ida::addLogMsg("Python auto-completer initialized successfully\n");

	Labeless& ll = Labeless::instance();
	while (ll.m_Enabled == 1)
	{
		QMutexLocker lock(&ll.m_AutoCompletionLock);
		while (!ll.m_AutoCompletionState->isValid() && ll.m_Enabled == 1)
		{
			ll.m_AutoCompletionCond.wait(&ll.m_AutoCompletionLock, waitTime);
		}
		if (ll.m_Enabled != 1)
			break;

		const auto request = *ll.m_AutoCompletionRequest;
		lock.unlock();

		QSharedPointer<jedi::Result> result(new jedi::Result);

		if (!util::python::jedi::get_completions(request.script, request.zline, request.zcol, result->completions, result->sigMatches, error))
		{
			util::ida::addLogMsg("jedi::get_completions() failed, error: %s\n", error.toStdString().c_str());
			lock.relock();
			ll.m_AutoCompletionState->state = jedi::State::RS_FINISHED;
			lock.unlock();
			emit completeError(error);
			continue;
		}

		lock.relock();
		ll.m_AutoCompletionResult = result;
		ll.m_AutoCompletionState->state = jedi::State::RS_FINISHED;
		lock.unlock();

		emit completeFinished(); // TODO
	}
}
