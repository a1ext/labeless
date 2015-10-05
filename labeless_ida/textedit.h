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

class Highlighter;
struct PythonPalette;

class TextEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit TextEdit(QWidget* parent = 0);
	~TextEdit();

	bool asHighlightedHtml(QString& result);
	void setPalette(const PythonPalette& p);

public slots:
	void colorSchemeChanged();

private slots:
	void insertCompletion(const QString& completion);

protected:
	virtual void focusInEvent(QFocusEvent* e);
	virtual void keyPressEvent(QKeyEvent* e);

private:
	QString textUnderCursor() const;

private:
	QPointer<Highlighter> m_Highlighter;
	QPointer<QCompleter> m_Completer;
};

