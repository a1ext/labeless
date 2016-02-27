/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once
#include <QDialog>

struct Settings;

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QTableWidgetItem)

struct PythonPalette;
enum PythonPaletteType;
enum PythonPaletteEntryType;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(const Settings& settings, qulonglong currModBase, QWidget* parent = nullptr);
	~SettingsDialog();

	void getSettings(Settings& result);

	bool validate() const;
	inline bool isPaletteChanged() const { return m_PaletteChanged; }
	bool getLightPalette(PythonPalette& result) const;
	bool getDarkPalette(PythonPalette& result) const;

signals:
	void testConnection();
	void syncronizeNow();
	void remotePythonExec();
	void pythonPaletteChanged();

protected:
	void changeEvent(QEvent* e);
	virtual void accept();

private slots:
	void on_bSyncNow_clicked();
	void on_bSaveAncClose_clicked();
	void on_bRemotePythonExecution_clicked();
	void on_twPyPaletteNames_currentItemChanged(QTableWidgetItem*, QTableWidgetItem*);
	void on_cbPaletteType_currentIndexChanged(int index);
	void on_bPalettePickColor_clicked();
	void on_chPaletteBold_toggled(bool value);
	void on_chPaletteItalic_toggled(bool value);
	void on_bResetPalette_clicked();

private:
	void setUpPalette();
	void updateCurrentPalette();
	PythonPaletteEntryType getSelectedPaletteEntryType();
	PythonPalette* getCurrentPalette();

private:
	Ui::SettingsDialog* m_UI;
	bool				m_PaletteChanged;
};
