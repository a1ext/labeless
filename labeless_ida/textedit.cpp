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
#include "util/util_python.h"
#include "jedi.h"
#include "labeless_ida.h"
#include "pysignaturetooltip.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStringListModel>
#include <QTimer>

namespace {

static const QString kkwExtern = "__extern__";
static const QString kkwResult = "__result__";
static const QString kkwResultStr = "__result_str__";
static const QStringList kAdditionalKeyWords = QStringList()
	<< "class" << "def" << "import" << "from" << "with" << "object" << "open";

} // anonymous


TextEdit::TextEdit(QWidget* parent)
	: QPlainTextEdit(parent)
	, m_CompletionModel(new QStringListModel(this))
	, m_CompletionTimer(new QTimer(this))
	, m_SignatureToolTip(new PySignatureToolTip(this))
	, m_CompletionType(CT_Unknown)
	, m_IsIDASide(false)
{
	m_LineNumberArea = new TELineNumberArea(this);
	CHECKED_CONNECT(connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int))));
	CHECKED_CONNECT(connect(this, SIGNAL(updateRequest(const QRect&, int)), this, SLOT(updateLineNumberArea(const QRect&, int))));

	m_Highlighter = new Highlighter(document());
	m_InternalNames
		<< kkwExtern << kkwResult;

	if (!util::python::jedi::is_available())
	{
		m_InternalNames += kAdditionalKeyWords;
	}
	m_CompletionModel->setStringList(m_InternalNames);

	m_Completer = new QCompleter(m_CompletionModel, this); // TODO: add support to load from file
	m_Completer->setCompletionMode(QCompleter::PopupCompletion);
	m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_Completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	m_Completer->setMaxVisibleItems(15);
	m_Completer->setWidget(this);
	CHECKED_CONNECT(connect(m_Completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString))));

	m_CompletionTimer->setSingleShot(true);
	m_CompletionTimer->setInterval(1200);
	CHECKED_CONNECT(connect(m_CompletionTimer, SIGNAL(timeout()), this, SLOT(onAutoCompletionRequested())));

	updateLineNumberAreaWidth(0);
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

void TextEdit::onAutoCompletionRequested()
{
	QSharedPointer<jedi::Request> req(new jedi::Request);
	req->script = textTillCursor();
	// std::string s = req->script.toStdString();
	QTextCursor tc = textCursor();
	req->zline = tc.blockNumber();
	req->zcol = tc.positionInBlock();

	emit autoCompleteThis(req);
}

void TextEdit::focusInEvent(QFocusEvent* e)
{
	if (m_Completer)
		m_Completer->setWidget(this);
	QPlainTextEdit::focusInEvent(e);
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

	if (m_CompletionTimer->isActive())
		m_CompletionTimer->stop();

	m_SignatureToolTip->hide();

	if (m_Completer && m_Completer->popup()->isVisible())
	{
		switch (e->key())
		{
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
		QPlainTextEdit::keyPressEvent(e);

	if (!isCompletionsRequested && e->text().isEmpty())
		return;
	
	const bool ctrlOrShiftOrAlt = e->modifiers() & (Qt::ControlModifier /*| Qt::ShiftModifier*/ | Qt::AltModifier);
	if (!m_Completer || (ctrlOrShiftOrAlt && e->text().isEmpty()))
		return;

	static const QString eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-=");
	const bool hasModifier = (e->modifiers() != Qt::NoModifier) && ctrlOrShiftOrAlt;
	QString completionPrefix = textUnderCursor();

	if (!isCompletionsRequested && (hasModifier || e->text().isEmpty() || completionPrefix.length() < kMinCompletionPrefixLen
						|| eow.contains(e->text().right(1))))
	{
		m_Completer->popup()->hide();
		return;
	}

	if (util::python::jedi::is_available())
	{
		m_CompletionType = isCallSignaturesRequested ? CT_CallSignature : CT_Completions;
		if (isCompletionsRequested || isCallSignaturesRequested)
		{
			// don't wait, just ask worker for completion
			onAutoCompletionRequested();
		}
		else
		{
			m_CompletionTimer->start();
		}
	}
	if (isCallSignaturesRequested)
		return;

	if (completionPrefix != m_Completer->completionPrefix())
	{
		m_Completer->setCompletionPrefix(completionPrefix);
		m_Completer->popup()->setCurrentIndex(m_Completer->completionModel()->index(0, 0));
	}
	
	QRect cr = cursorRect();
	cr.setWidth(m_Completer->popup()->sizeHintForColumn(0)
				+ m_Completer->popup()->verticalScrollBar()->sizeHint().width());
	m_Completer->complete(cr);
}

void TextEdit::focusOutEvent(QFocusEvent *e)
{
	if (m_CompletionTimer->isActive())
		m_CompletionTimer->stop();

	m_SignatureToolTip->hide();

	QPlainTextEdit::focusOutEvent(e);
}

void TextEdit::mousePressEvent(QMouseEvent* e)
{
	m_SignatureToolTip->hide();
	QPlainTextEdit::mousePressEvent(e);
}

void TextEdit::mouseMoveEvent(QMouseEvent* e)
{
	m_SignatureToolTip->hide();
	QPlainTextEdit::mouseMoveEvent(e);
}

void TextEdit::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	m_LineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEdit::colorSchemeChanged()
{
	auto palette = PythonPaletteManager::instance().palette();
	setPalette(palette);
}

void TextEdit::onAutoCompleteFinished(QSharedPointer<jedi::Result> r)
{
	if (!hasFocus())
		return;

	m_SignatureToolTip->hide();

	if (m_CompletionType == CT_CallSignature)
	{
		QStringList signatureList;
		if (!r->sigMatches.isEmpty())
		{
			foreach(const auto& sigMatch, r->sigMatches)
			{
				QStringList argList;
				for (int i = 0; i < sigMatch.args.count(); ++i)
				{
					if (i == sigMatch.argIndex)
						argList.append(QString("<b>%1</b>").arg(sigMatch.args.at(i).description));
					else
						argList.append(QString("%1").arg(sigMatch.args.at(i).description));
				}
				QString qsig = QString("%1(%2)").arg(sigMatch.name).arg(argList.join(", "));
				if (!sigMatch.rawDoc.isEmpty())
				{
					QStringList items = sigMatch.rawDoc.split(QRegExp("\\r|\\n"), QString::SkipEmptyParts);
					qsig += "<br><br>" + items.join("<br>");
				}
				//const std::string sig = qsig.toStdString();
				signatureList.append(qsig);
			}
			const QRect& cr = cursorRect();
			m_SignatureToolTip->showText(signatureList.join("<br>"), mapToGlobal(cr.bottomLeft()));
		}
		return;
	}

	//msg("%s: (REMOVE ME) completions got: %s\n-------------------\n",
	//	__FUNCTION__, r->completions.join("\n").toStdString().c_str());
	m_CompletionModel->setStringList(r->completions + m_InternalNames);

	const QString& completionPrefix = textUnderCursor();
	m_Completer->setCompletionPrefix(completionPrefix);
	m_Completer->popup()->setCurrentIndex(m_Completer->completionModel()->index(0, 0));

	QRect cr = cursorRect();
	cr.setWidth(m_Completer->popup()->sizeHintForColumn(0)
		+ m_Completer->popup()->verticalScrollBar()->sizeHint().width());
	m_Completer->complete(cr);
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

void TextEdit::markAsIDASideEditor(bool v)
{
	m_IsIDASide = v;

	if (m_IsIDASide)
	{
		m_InternalNames << kkwResultStr;
		CHECKED_CONNECT(connect(this, SIGNAL(autoCompleteThis(QSharedPointer<jedi::Request>)),
				&Labeless::instance(), SLOT(onAutoCompleteRequested(QSharedPointer<jedi::Request>))));
	}
	else
	{
		CHECKED_CONNECT(connect(this, SIGNAL(autoCompleteThis(QSharedPointer<jedi::Request>)),
			&Labeless::instance(), SLOT(onAutoCompleteRemoteRequested(QSharedPointer<jedi::Request>))));
	}
}

int TextEdit::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits; // workaround

	return space;
}

void TextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEdit::updateLineNumberArea(const QRect& rect, int dy)
{
	if (dy)
		m_LineNumberArea->scroll(0, dy);
	else
		m_LineNumberArea->update(0, rect.y(), m_LineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void TextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	QPainter painter(m_LineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, m_LineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		++blockNumber;
	}
}
