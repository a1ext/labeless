/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "highlighter.h"
#include "pythonpalettemanager.h"

#include <QList>
#include <QRegExp>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextLayout>

struct HighlightingRule
{
	QRegExp pattern;
	int index;
	PythonPaletteEntryType t;

	HighlightingRule(const QString& p = QString::null, int idx = 0, PythonPaletteEntryType t_ = PPET_Unknown)
		: pattern(QRegExp(p))
		, index(idx)
		, t(t_)
	{}
};

namespace {

static struct HighlightingRules
{
	QVector<HighlightingRule> rules;

	HighlightingRule triSingle;
	HighlightingRule triDouble;

	HighlightingRules()
	{
		QStringList keywordPatterns;
		keywordPatterns << "\\band\\b" << "\\bassert\\b" << "\\bbreak\\b" << "\\bclass\\b" << "\\bcontinue\\b" << "\\bdef\\b" <<
				"\\bdel\\b" << "\\belif\\b" << "\\belse\\b" << "\\bexcept\\b" << "\\bexec\\b" << "\\bfinally\\b" <<
				"\\bfor\\b" << "\\bfrom\\b" << "\\bglobal\\b" << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b" <<
				"\\bis\\b" << "\\blambda\\b" << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b" << "\\bprint\\b" <<
				"\\braise\\b" << "\\breturn\\b" << "\\btry\\b" << "\\bwhile\\b" << "\\byield\\b" <<
				"\\bas\\b" << "\\bwith\\b" << "\\b__extern__\\b";
		QStringList operators;
		operators << "==" << "!=" << "<" << "<=" << ">" << ">=" <<
				"\\+" << "-" << "\\*" << "/" << "//" << "\\%" << "\\*\\*" <<
				"\\+=" << "-=" << "\\*=" << "/=" << "\\%=" <<
				"\\^" << "\\|" << "\\&" << "\\~" << ">>" << "<<" << "=";

		QStringList braces;
		braces << "\\{" << "\\}" << "\\(" << "\\)" << "\\[" << "\\]";

		QStringList reserved;
		reserved << "\\bobject\\b" << "\\bsuper\\b" << "\\blen\\b" << "\\bNone\\b" << "\\bTrue\\b" << "\\bFalse\\b"
			<< "\\bException\\b" << "\\bbasestring\\b" << "\\bunicode\\b"
			<< "\\bstr\\b" << "\\bint\\b" << "\\blist\\b" << "\\bdict\\b" << "\\btuple\\b";

		foreach(const QString& pattern, keywordPatterns)
			rules.append(HighlightingRule(pattern, 0, PPET_Keyword));

		foreach(const QString& pattern, operators)
			rules.append(HighlightingRule(pattern, 0, PPET_Operator));

		foreach(const QString& pattern, reserved)
			rules.append(HighlightingRule(pattern, 0, PPET_Reserved));

		foreach(const QString& pattern, braces)
			rules.append(HighlightingRule(pattern, 0, PPET_Brace));

		rules.append(HighlightingRule("\\bself\\b", 0, PPET_Self));
		rules.append(HighlightingRule("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"", 0, PPET_String));
		rules.append(HighlightingRule("'[^'\\\\]*(\\\\.[^'\\\\]*)*'", 0, PPET_String));

		rules.append(HighlightingRule("\\bdef\\b\\s*(\\w+)", 1, PPET_Defclass));
		rules.append(HighlightingRule("\\bclass\\b\\s*(\\w+)", 1, PPET_Defclass));

		rules.append(HighlightingRule("\\b[+-]?[0-9]+[lL]?\\b", 0, PPET_Number));
		rules.append(HighlightingRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0, PPET_Number));
		rules.append(HighlightingRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0, PPET_Number));
		rules.append(HighlightingRule("^[\\s]*(#[^\\n]*)", 0, PPET_Comment));

		triSingle = HighlightingRule("'''", 1, PPET_String2);
		triDouble = HighlightingRule("\"\"\"", 2, PPET_String2);
	}
} kHighlightingRules;

} // anonymous

Highlighter::Highlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent)
	, m_Palette(std::make_shared<PythonPalette>(PythonPaletteManager::instance().palette()))
{
	updatePalette();
}

void Highlighter::highlightBlock(const QString &text)
{
	foreach(const ::HighlightingRule& rule, kHighlightingRules.rules)
	{
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);
		while (index >= 0)
		{
			index = expression.pos(rule.index);
			int length = expression.cap(rule.index).length();
			QTextCharFormat fmt;
			if (m_Formats.contains(rule.t))
				fmt = m_Formats[rule.t];
			setFormat(index, length, fmt);
			index = expression.indexIn(text, index + length);
		}
	}

	setCurrentBlockState(0);

	bool isMultiline = matchMultiline(text, kHighlightingRules.triSingle);
	if (!isMultiline)
		isMultiline = matchMultiline(text, kHighlightingRules.triDouble);
}

bool Highlighter::matchMultiline(const QString& text, const ::HighlightingRule& rule)
{
	QRegExp delimiter(rule.pattern);
	int start = 0;
	int add = 0;
	int end = 0;
	int length = 0;
	if (previousBlockState() != rule.index)
	{
		start = delimiter.indexIn(text);
		add = delimiter.matchedLength();
	}

	while (start >= 0)
	{
		end = delimiter.indexIn(text, start + add);
		if (end >= add)
		{
			length = end - start + add + delimiter.matchedLength();
			setCurrentBlockState(0);
		}
		else
		{
			setCurrentBlockState(rule.index);
			length = text.length() - start + add;
		}
		QTextCharFormat fmt;
		if (m_Formats.contains(rule.t))
			fmt = m_Formats[rule.t];
		setFormat(start, length, fmt);
		start = delimiter.indexIn(text, start + length);
	}
	return currentBlockState() == rule.index;
}

void Highlighter::updatePalette(bool invalidate)
{
	static const QList<PythonPaletteEntryType> kTypes = QList<PythonPaletteEntryType>()
		<< PPET_Keyword << PPET_Operator << PPET_Reserved << PPET_Brace << PPET_Defclass << PPET_String << PPET_String2
		<< PPET_Comment << PPET_Self << PPET_Number;
	if (invalidate)
		*m_Palette = PythonPaletteManager::instance().palette();

	foreach(PythonPaletteEntryType t, kTypes)
	{
		m_Formats[t] = getTextCharFormat(t, *m_Palette);
	}
	rehighlight();
}

bool Highlighter::asHtml(QString& result)
{
	QTextCursor cursor(document());
	cursor.select(QTextCursor::Document);
	QSharedPointer<QTextDocument> tempDocument(new QTextDocument);

	QTextCursor tempCursor(tempDocument.data());

	tempCursor.insertFragment(cursor.selection());
	tempCursor.select(QTextCursor::Document);
	// Set the default foreground for the inserted characters.
	QTextCharFormat textfmt = tempCursor.charFormat();
	textfmt.setBackground(Qt::white);
	textfmt.setForeground(Qt::gray);
	tempCursor.setCharFormat(textfmt);

	// Apply the additional formats set by the syntax highlighter
	QTextBlock start = document()->findBlock(cursor.selectionStart());
	QTextBlock end = document()->findBlock(cursor.selectionEnd());
	end = end.next();
	const int selectionStart = cursor.selectionStart();
	const int endOfDocument = tempDocument->characterCount() - 1;
	for (QTextBlock current = start; current.isValid() && current != end; current = current.next()) {
		const QTextLayout* layout(current.layout());

		foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
			const int start = current.position() + range.start - selectionStart;
			const int end = start + range.length;
			if (end <= 0 || start >= endOfDocument)
				continue;
			tempCursor.setPosition(qMax(start, 0));
			tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
			tempCursor.setCharFormat(range.format);
		}
	}

	// Reset the user states since they are not interesting
	for (QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
		block.setUserState(-1);

	// Make sure the text appears pre-formatted, and set the background we want.
	tempCursor.select(QTextCursor::Document);
	QTextBlockFormat blockFormat = tempCursor.blockFormat();
	blockFormat.setNonBreakableLines(true);
	blockFormat.setBackground(Qt::white); // TODO: set background color of widget
	tempCursor.setBlockFormat(blockFormat);

	// Finally retrieve the syntax highlighted and formatted html.
	result = tempCursor.selection().toHtml();
	return true;
}

void Highlighter::setPalette(const PythonPalette& p)
{
	*m_Palette = p;
	updatePalette();
}

