/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <memory>

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QMap>

QT_FORWARD_DECLARE_CLASS(QTextDocument)

struct HighlightingRule;
struct PythonPalette;

class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	explicit Highlighter(QTextDocument* parent = nullptr);

	bool asHtml(QString& result);
	void setPalette(const PythonPalette& p);

public slots:
	void updatePalette(bool invalidate = false);

protected:
	void highlightBlock(const QString& text);

private:
	bool matchMultiline(const QString& text, const ::HighlightingRule& rule);

private:
	QMap<int, QTextCharFormat>		m_Formats;
	std::shared_ptr<PythonPalette>	m_Palette;
};
