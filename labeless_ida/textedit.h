/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <QPointer>
#include <QPlainTextEdit>

#include "jedi.h"

QT_FORWARD_DECLARE_CLASS(QCompleter)
QT_FORWARD_DECLARE_CLASS(QStringListModel)
QT_FORWARD_DECLARE_CLASS(QTimer)

class Highlighter;
class PySignatureToolTip;
struct PythonPalette;
class TELineNumberArea;

class TextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit TextEdit(QWidget* parent = 0);
	~TextEdit();

	bool asHighlightedHtml(QString& result);
	void setPalette(const PythonPalette& p);
	void markAsIDASideEditor(bool v);
	inline bool isIDASideEditor() const { return m_IsIDASide; }

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

signals:
	void autoCompleteThis(QSharedPointer<jedi::Request>);

public slots:
	void colorSchemeChanged();
	void onAutoCompleteFinished(QSharedPointer<jedi::Result> r);
	void selectLine(int line);

private slots:
	void insertCompletion(const QString& completion);
	void onAutoCompletionRequested();
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect&, int);

protected:
	void focusInEvent(QFocusEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
	void focusOutEvent(QFocusEvent *e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void resizeEvent(QResizeEvent *event) override;

private:
	QString textUnderCursor() const;
	QString textTillCursor() const;

private:
	enum CompletionType
	{
		CT_Unknown,
		CT_Completions,
		CT_CallSignature
	};

	QPointer<Highlighter> m_Highlighter;
	QPointer<QCompleter> m_Completer;
	QPointer<QStringListModel> m_CompletionModel;
	QStringList m_InternalNames;
	QPointer<QTimer> m_CompletionTimer;
	QPointer<PySignatureToolTip> m_SignatureToolTip;
	CompletionType m_CompletionType;
	QPointer<TELineNumberArea> m_LineNumberArea;

	bool m_IsIDASide;
};

class TELineNumberArea : public QWidget
{
public:
	TELineNumberArea(TextEdit *editor)
		: QWidget(editor)
	{
		m_Editor = editor;
	}

	QSize sizeHint() const override
	{
		return QSize(m_Editor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		m_Editor->lineNumberAreaPaintEvent(event);
	}

private:
	TextEdit* m_Editor;
};