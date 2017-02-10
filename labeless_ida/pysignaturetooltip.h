/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#pragma once

#include <QLabel>

class PySignatureToolTip : public QLabel
{
	Q_OBJECT
public:
	explicit PySignatureToolTip(QWidget* parent = nullptr);

	void showText(const QString& text, const QPoint& pos);

protected:
	void paintEvent(QPaintEvent *ev) override;
	void resizeEvent(QResizeEvent *e) override;
};