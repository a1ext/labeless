/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include <QList>
#include <QMetaType>
#include <QSharedPointer>
#include <QStringList>

struct extlang_t;

namespace jedi {

struct FuncArg
{
	QString name;
	QString description;
};
typedef QList<FuncArg> FuncArgList;

struct SignatureMatch
{
	QString type;
	QString name;
	int argIndex;
	FuncArgList args;
	QString rawDoc;

	SignatureMatch();
};

typedef QList<SignatureMatch> SignatureMatchList;

struct State
{
	enum StateType
	{
		RS_DONE,
		RS_ASKED,
		RS_FINISHED
	};

	StateType state;

	State();

	inline bool isValid() const	{ return state == RS_ASKED; }
	inline bool isFinished() const { return state == RS_FINISHED; }
};

struct Request
{
	QString script;
	quint32 zline;
	quint32 zcol;
	
	QObject* rcv;

	Request();
};

struct Result
{
	QStringList completions;
	SignatureMatchList sigMatches;
};

} // jedi

Q_DECLARE_METATYPE(QSharedPointer<jedi::Request>)
Q_DECLARE_METATYPE(QSharedPointer<jedi::Result>)

