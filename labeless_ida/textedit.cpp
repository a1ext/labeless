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
#include "pythonpalettemanager.h"
#include "jedi.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringListModel>


TextEdit::TextEdit(QWidget* parent)
	: QTextEdit(parent)
	, m_CompletionModel(new QStringListModel(this))
{
	m_Highlighter = new Highlighter(document());
	m_InternalNames
		<< "__extern__" << "__result__" << "__result_str__";

	if (!jedi::is_available())
	{
		m_InternalNames
			<< "class" << "def" << "import" << "from" << "with" << "object" << "open";
	}
	m_CompletionModel->setStringList(m_InternalNames);

	m_Completer = new QCompleter(m_CompletionModel, this); // TODO: add support to load from file
	m_Completer->setCompletionMode(QCompleter::PopupCompletion);
	m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_Completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	m_Completer->setMaxVisibleItems(15);
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
	tc.movePosition(QTextCursor::StartOfWord);
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	tc.removeSelectedText();
	tc.insertText(completion);
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

QString TextEdit::textTillCursor() const
{
	QTextCursor tc = textCursor();
	QString rv;
	for (int i = 0; i < tc.blockNumber(); ++i)
		rv += tc.document()->findBlockByNumber(i).text() + QLatin1Char('\n');
	rv += tc.document()->findBlockByNumber(tc.blockNumber()).text().mid(0, tc.positionInBlock());
	return rv;
}

void TextEdit::keyPressEvent(QKeyEvent* e)
{
	static const int kMinCompletionPrefixLen = 1; // 3

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

	const bool isCompletionsRequested = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);
	const bool isCallSignaturesRequested = isCompletionsRequested && (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
	if (!m_Completer || !isCompletionsRequested)
		QTextEdit::keyPressEvent(e);

	if (!isCompletionsRequested && e->text().isEmpty())
		return;
	
	const bool ctrlOrShiftOrAlt = e->modifiers() & (Qt::ControlModifier /*| Qt::ShiftModifier*/ | Qt::AltModifier);
	if (!m_Completer || (ctrlOrShiftOrAlt && e->text().isEmpty()))
		return;

	static const QString eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-=");
	const bool hasModifier = (e->modifiers() != Qt::NoModifier) && ctrlOrShiftOrAlt;
	QString completionPrefix = textUnderCursor();

	if (!isCompletionsRequested && (hasModifier || e->text().isEmpty() || completionPrefix.length() < kMinCompletionPrefixLen
						|| eow.contains(e->text().right(1)))) {
		m_Completer->popup()->hide();
		return;
	}

	if (jedi::is_available())
	{
		const QString textBeforeCursor = textTillCursor();
		//std::string s = textBeforeCursor.toStdString();
		QString error;
		QTextCursor tc = textCursor();
		const int row = tc.blockNumber();
		const int col = tc.positionInBlock();
		QStringList completions;
		jedi::SignatureMatchList sigMatches;
		msg("%s: -------------------------------------\n", __FUNCTION__);
		QElapsedTimer t;
		t.start();
		const bool ok = jedi::get_completions(textBeforeCursor, row, col, completions, sigMatches, error);
		const int nMsec = t.elapsed();
		msg("%s: jedi::get_completions() took %s\n", __FUNCTION__, QDateTime::fromMSecsSinceEpoch(nMsec).toUTC().toString("hh:mm:ss.zzz").toStdString().c_str());
		if (!ok)
		{
			msg("%s: jedi::get_completions() failed\n", __FUNCTION__);
			m_CompletionModel->setStringList(m_InternalNames);
		}
		else
		{
			QStringList signatureList;
			if (!sigMatches.isEmpty())
			{
				for (const auto& sigMatch : sigMatches)
				{
					QStringList argList;
					for (int i = 0; i < sigMatch.args.count(); ++i)
					{
						if (i == sigMatch.argIndex)
							argList.append(QString("<b>%1</b>").arg(sigMatch.args.at(i).description));
						else
							argList.append(QString("%1").arg(sigMatch.args.at(i).description));
					}
					const QString qsig = QString("%1(%2)").arg(sigMatch.name).arg(argList.join(", "));
					const std::string sig = qsig.toStdString();
					signatureList.append(qsig);
				}
			}
			// TODO: use signatureList
			m_CompletionModel->setStringList(m_InternalNames + completions);
		}
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
