/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "textedit.h"
#include "types.h"
#include "highlighter.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QScrollBar>

#include "pythonpalettemanager.h"

TextEdit::TextEdit(QWidget* parent)
	: QTextEdit(parent)
{
	m_Highlighter = new Highlighter(document());
	m_Completer = new QCompleter(QStringList()
		<< "class"<< "def"<< "import"<< "from"<< "with" << "__extern__" << "__result__" << "__result_str__", this); // TODO: add support to load from file
	m_Completer->setCompletionMode(QCompleter::PopupCompletion);
	m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_Completer->setWidget(this);
	CHECKED_CONNECT(connect(m_Completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString))));

	setAcceptRichText(false);
}

TextEdit::~TextEdit()
{
}

void TextEdit::insertCompletion(const QString& completion)
{
	if (m_Completer->widget() != this)
		return;
	QTextCursor tc = textCursor();
	int extra = completion.length() - m_Completer->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(extra));
	setTextCursor(tc);
}
void TextEdit::focusInEvent(QFocusEvent* e)
{
	if (m_Completer)
		m_Completer->setWidget(this);
	QTextEdit::focusInEvent(e);
}

QString TextEdit::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void TextEdit::keyPressEvent(QKeyEvent* e)
{
	if (m_Completer && m_Completer->popup()->isVisible()) {
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return;
		default:
			break;
		}
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);
	if (!m_Completer || !isShortcut)
		QTextEdit::keyPressEvent(e);

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!m_Completer || (ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
						|| eow.contains(e->text().right(1)))) {
		m_Completer->popup()->hide();
		return;
	}

	if (completionPrefix != m_Completer->completionPrefix()) {
		m_Completer->setCompletionPrefix(completionPrefix);
		m_Completer->popup()->setCurrentIndex(m_Completer->completionModel()->index(0, 0));
	}
	QRect cr = cursorRect();
	cr.setWidth(m_Completer->popup()->sizeHintForColumn(0)
				+ m_Completer->popup()->verticalScrollBar()->sizeHint().width());
	m_Completer->complete(cr);
}

void TextEdit::colorSchemeChanged()
{
	auto palette = PythonPaletteManager::instance().palette();
	setPalette(palette);
}

bool TextEdit::asHighlightedHtml(QString& result)
{
	result.clear();
	if (m_Highlighter)
		return m_Highlighter->asHtml(result);
	return false;
}

void TextEdit::setPalette(const PythonPalette& p)
{
	QFont fnt(p.mainFont, p.mainFontPointSize);
	setFont(fnt);

	auto w = p.tabWidth * fontMetrics().width(' ');
	setTabStopWidth(w);

	if (m_Highlighter)
		m_Highlighter->setPalette(p);
}
