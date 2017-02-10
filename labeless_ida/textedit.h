/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <QPointer>
#include <QTextEdit>

QT_FORWARD_DECLARE_CLASS(QCompleter)
QT_FORWARD_DECLARE_CLASS(QStringListModel)
QT_FORWARD_DECLARE_CLASS(QTimer)

class Highlighter;
class PySignatureToolTip;
struct PythonPalette;

namespace jedi {
struct Result;
struct Request;
} // jedi

class TextEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit TextEdit(QWidget* parent = 0);
	~TextEdit();

	bool asHighlightedHtml(QString& result);
	void setPalette(const PythonPalette& p);

signals:
	void autoCompleteThis(QSharedPointer<jedi::Request>);

public slots:
	void colorSchemeChanged();
	void onAutoCompleteFinished(QSharedPointer<jedi::Result> r);

private slots:
	void insertCompletion(const QString& completion);
	void onAutoCompletionRequested();

protected:
	void focusInEvent(QFocusEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
	void focusOutEvent(QFocusEvent *e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

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
};

