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
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextLayout>


struct HighlightingRule
{
	QRegularExpression pattern;
	int index;
	PythonPaletteEntryType t;

	HighlightingRule(const QString& p = QString(), int idx = 0, PythonPaletteEntryType t_ = PPET_Unknown)
		: pattern(p)
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
				"\\bas\\b" << "\\bwith\\b" << "\\b__extern__\\b" << "\\b__result_str__\\b" << "\\b__result__\\b";
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
		rules.append(HighlightingRule("[\\s]*(#[^\\n]*)", 0, PPET_Comment));

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
	QTextCharFormat fmt;
	fmt.setFont(QFont(m_Palette->mainFont, m_Palette->mainFontPointSize));
	setFormat(0, text.length(), fmt);

	foreach(const ::HighlightingRule& rule, kHighlightingRules.rules)
	{
		applyCbOnMatchOf(rule, text, [this](int index, int length, const QTextCharFormat& tf) {
			setFormat(index, length, tf);
		});
	}

	setCurrentBlockState(0);

	bool isMultiline = matchMultiline(text, kHighlightingRules.triSingle);
	if (!isMultiline)
		isMultiline = matchMultiline(text, kHighlightingRules.triDouble);
}


bool Highlighter::matchMultiline(const QString& text, const ::HighlightingRule& rule)
{
	QRegularExpression delimiter(rule.pattern);
	int start = 0, add = 0, end = 0, length = 0;
	QRegularExpressionMatch match;

	if (previousBlockState() != rule.index)
	{
		match = delimiter.match(text);
		start = match.hasMatch() ? match.capturedStart() : -1;
		add = match.hasMatch() ? match.capturedLength() : 0;
	}

	while (start >= 0)
	{
		match = delimiter.match(text, start + add);
		end = match.hasMatch() ? match.capturedStart() : -1;
		if (end >= add)
		{
			length = end - start + add + match.capturedLength();
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
		match = delimiter.match(text, start + length);
		start = match.hasMatch() ? match.capturedStart() : -1;
	}
	return currentBlockState() == rule.index;
}

void Highlighter::applyCbOnMatchOf(const ::HighlightingRule& rule, const QString& text, std::function<void(int, int, const QT::QTextCharFormat&)> cb)
{
	QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
	while (matchIterator.hasNext())
	{
		QRegularExpressionMatch match = matchIterator.next();
		const int index = match.capturedStart(rule.index);
		const int length = match.capturedLength(rule.index);
		if (m_Formats.contains(rule.t))
			cb(index, length, m_Formats[rule.t]);
	}
}

void Highlighter::updatePalette(bool invalidate)
{
	static const QList<PythonPaletteEntryType> kTypes = QList<PythonPaletteEntryType>()
		<< PPET_Keyword << PPET_Operator << PPET_Reserved << PPET_Brace << PPET_Defclass << PPET_String << PPET_String2
		<< PPET_Comment << PPET_Self << PPET_Number << PPET_Highlight;
	if (invalidate)
		*m_Palette = PythonPaletteManager::instance().palette();

	foreach(PythonPaletteEntryType t, kTypes)
	{
		m_Formats[t] = getTextCharFormat(t, *m_Palette);
	}
	document()->setDefaultFont(QFont(m_Palette->mainFont, m_Palette->mainFontPointSize));
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

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
		const auto& fmts = layout->formats();
#else // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
		const auto& fmts = layout->additionalFormats();
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

		foreach(const auto& range, fmts) 
		{
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

