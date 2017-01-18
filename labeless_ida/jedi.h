#pragma once

#include <QList>
#include <QString>

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
};

typedef QList<SignatureMatch> SignatureMatchList;


bool init_completer(const extlang_t* elng, QString& error);

bool is_available();

bool get_completions(const QString& script,
	int zline,
	int zcol,
	QStringList& completions,
	SignatureMatchList& signatureMatches,
	QString& error);


} // jedi

