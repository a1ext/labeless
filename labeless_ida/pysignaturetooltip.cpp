/* Labeless
* by Aliaksandr Trafimchuk
*
* Source code released under
* Creative Commons BY-NC 4.0
* http://creativecommons.org/licenses/by-nc/4.0
*/

#include "pysignaturetooltip.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QToolTip>

namespace {

int getTipScreen(const QPoint &pos, QWidget *w)
{
    if (QApplication::desktop()->isVirtualDesktop())
        return QApplication::desktop()->screenNumber(pos);
    return QApplication::desktop()->screenNumber(w);
}

enum
{
	MAX_TOOLTIP_WIDTH = 600
};

} // anonymous


PySignatureToolTip::PySignatureToolTip(QWidget* parent /*= nullptr*/)
	: QLabel(parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	setPalette(QToolTip::palette());

	const int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
	setMargin(margin);

	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft);
	setIndent(1);

	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / qreal(255.0));
	setMouseTracking(true);

	setMaximumWidth(MAX_TOOLTIP_WIDTH);
	setWordWrap(true);

	hide();
}

void PySignatureToolTip::showText(const QString& text, const QPoint& pos)
{
	// grabbed from Qt sources qtooltip.cpp
	// 
	setText(text);
	QFontMetrics fm(font());
	QSize extra(1, 0);
	// Make it look good with the default ToolTip font on Mac, which has a small descent.
	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();
	
	resize(sizeHint() + extra);

#ifdef Q_WS_MAC
	// When in full screen mode, there is no Dock nor Menu so we can use
	// the whole screen for displaying the tooltip. However when not in
	// full screen mode we need to save space for the dock, so we use
	// availableGeometry instead.
	extern bool qt_mac_app_fullscreen; //qapplication_mac.mm
	QRect screen;
	if (qt_mac_app_fullscreen)
		screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
	else
		screen = QApplication::desktop()->availableGeometry(getTipScreen(pos, w));
#else
	QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, parentWidget()));
#endif

	QPoint p = pos - QPoint(0, 3);
	p += QPoint(2,
#ifdef Q_WS_WIN
		21
#else
		16
#endif
	);
	if (p.x() + this->width() > screen.x() + screen.width())
		p.rx() -= 4 + this->width();
	if (p.y() + this->height() > screen.y() + screen.height())
		p.ry() -= 24 + this->height();
	if (p.y() < screen.y())
		p.setY(screen.y());
	if (p.x() + this->width() > screen.x() + screen.width())
		p.setX(screen.x() + screen.width() - this->width());
	if (p.x() < screen.x())
		p.setX(screen.x());
	if (p.y() + this->height() > screen.y() + screen.height())
		p.setY(screen.y() + screen.height() - this->height());

	move(p);
	show();
}

void PySignatureToolTip::paintEvent(QPaintEvent *ev)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.init(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();

	QLabel::paintEvent(ev);
}

void PySignatureToolTip::resizeEvent(QResizeEvent *e)
{
	QStyleHintReturnMask frameMask;
	QStyleOption option;
	option.init(this);
	if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
		setMask(frameMask.region);

	QLabel::resizeEvent(e);
}
