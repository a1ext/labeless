/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <QtGlobal>
#include <QMap>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#   include <QtWidgets/QWidget>
#else // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#   include <QWidget>
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

QT_FORWARD_DECLARE_CLASS(QCloseEvent)

QT_BEGIN_NAMESPACE
namespace Ui {
class PyOllyView;
}
QT_END_NAMESPACE

class PyOllyView : public QWidget
{
	Q_OBJECT

public:
	explicit PyOllyView(bool isShowAllResponsesInLog, QWidget* parent = nullptr);
	~PyOllyView();

	void prependStdoutLog(const QString& text, bool isHtml = false);
	void prependStderrLog(const QString& text, bool isHtml = false);

	QString getOllyScript(bool html = false) const;
	QString getIDAScript(bool html = false) const;

public slots:
	void enableRunScriptButton(bool enabled);
	void setOllyScript(const QString& text);
	void setIDAScript(const QString& text);
	void onColorSchemeChanged();

private slots:
	void onScriptEditContextMenuRequested(const QPoint& pt);
	void onLoadTemplateRequested();
	void onFastActionRequested();
	void onBindScriptsToFastActionRequested();
	void onClearBindingScriptsOfFastAction();

private:
	void setUpConnections();
	void setUpGUI();

signals:
	void showAllResponsesInLogToggled(bool);

	void runScriptRequested();
	void settingsRequested();
	void anchorClicked(const QString&);
	void clearLogsRequested();

protected:
	virtual void changeEvent(QEvent* e);
	virtual bool eventFilter(QObject* obj, QEvent* event);

private:
	Ui::PyOllyView* m_UI;
	QMap<QString, QString> m_OllyTemplates;
	QMap<QString, QString> m_IDATemplates;
};
