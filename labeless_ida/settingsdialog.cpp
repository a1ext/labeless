/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QColorDialog>
#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTextCharFormat>

#include "types.h"
#include "globalsettingsmanager.h"
#include "pythonpalettemanager.h"
#include "util/util_ida.h"
#include "util/util_idapython.h"
#include "util/util_python.h"
#include "../common/version.h"


namespace {

enum PaletteItemRole
{
	PIR_PaletteType = Qt::UserRole,
	PIR_PPalette
};
static const std::string kPropColor = "p_color";

} // anonymous

SettingsDialog::SettingsDialog(const Settings& settings, qulonglong currModBase, QWidget* parent)
	: QDialog(parent)
	, m_UI(new Ui::SettingsDialog)
	, m_PaletteChanged(false)
	, m_bIgnoreChange(0)
{
	m_UI->setupUi(this);
	CHECKED_CONNECT(connect(m_UI->bTestConnection, SIGNAL(clicked()), this, SIGNAL(testConnection())));
	CHECKED_CONNECT(connect(m_UI->bDiscard, SIGNAL(clicked()), this, SLOT(reject())));

	const QVariantList prevHosts = GlobalSettingsManger::instance().
			value(GSK_PrevEnteredOllyHosts).toList();

	m_UI->cbOllyIP->clear();
	foreach(const QVariant& v, prevHosts)
	{
		const QString sv = v.toString().trimmed();
		if (sv.isEmpty())
			continue;
		m_UI->cbOllyIP->addItem(sv);
	}

	const QString currHostVal = QString::fromStdString(settings.host);
	int currHostIdx = m_UI->cbOllyIP->findText(currHostVal);
	if (currHostIdx == -1)
	{
		m_UI->cbOllyIP->insertItem(0, currHostVal);
	}
	m_UI->cbOllyIP->setCurrentIndex(m_UI->cbOllyIP->findText(currHostVal));

	m_UI->sbOllyPort->setValue(settings.port);
	m_UI->leRemoteModuleBase->setText(QString("0x%1").arg(settings.remoteModBase, sizeof(ea_t) * 2, 16, QChar('0')));
	m_UI->leRemoteModuleBase->setToolTip(QString("Current IDA DB's module base is 0x%1.").arg(currModBase, sizeof(ea_t) * 2, 16, QChar('0')));
	m_UI->cbAutoSync->setChecked(settings.enabled);
	m_UI->chDemangleNames->setChecked(settings.demangle);
	m_UI->chLocalLabels->setChecked(settings.localLabels);
	m_UI->chPerformPEAnalysis->setChecked(settings.analysePEHeader);
	m_UI->chPostProcessFixCallJumps->setChecked(settings.postProcessFixCallJumps);
	m_UI->chNonCodeNames->setChecked(settings.nonCodeNames);
	m_UI->chRemoveFuncArgs->setChecked(settings.removeFuncArgs);
	m_UI->cbOverwriteWarning->setCurrentIndex(settings.overwriteWarning);
	m_UI->chIDAComments->setChecked(settings.commentsSync.testFlag(Settings::CS_IDAComment));
	m_UI->chFuncLocalVars->setChecked(settings.commentsSync.testFlag(Settings::CS_LocalVar));
	m_UI->chFuncNameAsComment->setChecked(settings.commentsSync.testFlag(Settings::CS_FuncNameAsComment));
	m_UI->chFuncLocalVarsAll->setChecked(settings.commentsSync.testFlag(Settings::CS_LocalVarAll));

	QLabel* const lVer = new QLabel(m_UI->tabCommon);
	lVer->setText(QString("v %1").arg(LABELESS_VER_STR));
	lVer->setStyleSheet("color: rgb(0x8a, 0x8a, 0x8a);");
	QGraphicsDropShadowEffect* const effect = new QGraphicsDropShadowEffect(lVer);
	effect->setBlurRadius(5);
	effect->setOffset(0);
	effect->setColor(Qt::white);
	lVer->setGraphicsEffect(effect);
	m_UI->tw->setCornerWidget(lVer);
	m_UI->fcbFont->setFontFilters(QFontComboBox::MonospacedFonts);

	m_UI->bgAutoCompletion->setChecked(settings.codeCompletion);
	const bool jediAvailable = util::python::jedi::is_available();
	m_UI->lbJediStatus->setText(QString("<p style=\"color: %1\">%2</p>").arg(jediAvailable ? "green": "red").arg(jediAvailable ? tr("available") : tr("not available")));

	setUpPalette();
	//adjustSize();
	setMaximumSize(size());
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_UI->tw->setCurrentIndex(0);
}

SettingsDialog::~SettingsDialog()
{
	QTableWidgetItem* const itLight = m_UI->twPyPaletteNames->item(0, 0);
	QTableWidgetItem* const itDark = m_UI->twPyPaletteNames->item(1, 0);
	if (itLight)
	{
		if (PythonPalette* const lightPalette = static_cast<PythonPalette*>(itLight->data(PIR_PPalette).value<void*>()))
			delete lightPalette;
	}
	if (!itDark)
	{
		if (PythonPalette* const darkPalette = static_cast<PythonPalette*>(itLight->data(PIR_PPalette).value<void*>()))
			delete darkPalette;
	}

	delete m_UI;
}

void SettingsDialog::setUpPalette()
{
	static const struct {
		QString name;
		PythonPaletteEntryType type;
	} kPaletteEntryTypes[] = {
		{ tr("Keyword"),										PPET_Keyword, },
		{ tr("Operator"),										PPET_Operator },
		{ tr("Reserved"),										PPET_Reserved },
		{ tr("Brace"),											PPET_Brace },
		{ tr("Def/Class"),										PPET_Defclass },
		{ tr("String between ' and \""),						PPET_String },
		{ tr("String between triple quotes '''' and \"\"\""),	PPET_String2 },
		{ tr("Comment"),										PPET_Comment },
		{ tr("Self"),											PPET_Self },
		{ tr("Number"),											PPET_Number },
		{ tr("Highlight bground"),								PPET_Highlight }
	};

	QTableWidgetItem* const light = new QTableWidgetItem(tr("light"));
	light->setData(PIR_PaletteType, PPT_Light);
	auto pLightPalette = new PythonPalette(PythonPaletteManager::instance().lightPalette());
	light->setData(PIR_PPalette, QVariant::fromValue<void*>(pLightPalette));

	QTableWidgetItem* const dark = new QTableWidgetItem(tr("dark"));
	dark->setData(PIR_PaletteType, PPT_Dark);
	auto pDarkPalette = new PythonPalette(PythonPaletteManager::instance().darkPalette());
	dark->setData(PIR_PPalette, QVariant::fromValue<void*>(pDarkPalette));

	for (unsigned i = 0; i < _countof(kPaletteEntryTypes); ++i)
		m_UI->cbPaletteType->addItem(kPaletteEntryTypes[i].name, kPaletteEntryTypes[i].type);

	m_UI->twPyPaletteNames->setRowCount(2);
	m_UI->twPyPaletteNames->setItem(0, 0, light);
	m_UI->twPyPaletteNames->setItem(1, 0, dark);

	QFont fnt(pLightPalette->mainFont);
	fnt.setPointSize(pLightPalette->mainFontPointSize);

	m_UI->cbPaletteType->setCurrentIndex(0);
	m_UI->twPyPaletteNames->setCurrentItem(light);
}

void SettingsDialog::getSettings(Settings& result)
{
	result = Settings();
	result.host = m_UI->cbOllyIP->currentText().toStdString();
	result.port = static_cast<short>(m_UI->sbOllyPort->value());
	bool ok = false;
	QString remoteModBase = m_UI->leRemoteModuleBase->text().trimmed();
	result.remoteModBase = remoteModBase.toULongLong(&ok, 16);
	
	result.enabled = m_UI->cbAutoSync->isChecked();
	result.demangle = m_UI->chDemangleNames->isChecked();
	result.localLabels = m_UI->chLocalLabels->isChecked();
	result.nonCodeNames = m_UI->chNonCodeNames->isChecked();
	result.analysePEHeader = m_UI->chPerformPEAnalysis->isChecked();
	result.postProcessFixCallJumps = m_UI->chPostProcessFixCallJumps->isChecked();
	result.removeFuncArgs = m_UI->chRemoveFuncArgs->isChecked();
	result.overwriteWarning = static_cast<Settings::OverwriteWarning>(m_UI->cbOverwriteWarning->currentIndex());
	result.commentsSync = Settings::CS_Disabled;
	if (m_UI->chIDAComments->isChecked())
		result.commentsSync |= Settings::CS_IDAComment;
	if (m_UI->chFuncLocalVars->isChecked())
		result.commentsSync |= Settings::CS_LocalVar;
	if (m_UI->chFuncNameAsComment->isChecked())
		result.commentsSync |= Settings::CS_FuncNameAsComment;
	if (m_UI->chFuncLocalVarsAll->isChecked())
		result.commentsSync |= Settings::CS_LocalVarAll;
	result.codeCompletion = m_UI->bgAutoCompletion->isChecked();
}

void SettingsDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		m_UI->retranslateUi(this);
		break;
	default:
		break;
	}
}

void SettingsDialog::on_bSyncNow_clicked()
{
	if (!validate())
		return;
	emit syncronizeNow();
	accept();
}

void SettingsDialog::on_bSaveAncClose_clicked()
{
	if (!validate())
		return;
	accept();
}

void SettingsDialog::on_bRemotePythonExecution_clicked()
{
	if (!validate())
		return;
	emit remotePythonExec();
	accept();
}

void SettingsDialog::accept()
{
	QVariantList vl;
	const int hostCurrIdx = m_UI->cbOllyIP->currentIndex();
	if (hostCurrIdx == -1)
	{
		vl.append(m_UI->cbOllyIP->currentText());
	}

	QSet<QString> prevItems;
	for (int i = 0; i < m_UI->cbOllyIP->count(); ++i)
	{
		auto str = m_UI->cbOllyIP->itemText(i);
		if (prevItems.contains(str))
			continue;
		prevItems.insert(str);
		vl.append(str);
	}
	GlobalSettingsManger::instance().setValue(GSK_PrevEnteredOllyHosts, QVariant(vl));
	QDialog::accept();
}

bool SettingsDialog::validate() const
{
	static const QRegExp kRxHostname("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$");
	static const QRegExp kRxIpAddress("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");

	const QString hostOrIP = m_UI->cbOllyIP->currentText();
	if (!kRxIpAddress.exactMatch(hostOrIP) && !kRxHostname.exactMatch(hostOrIP))
	{
		info(tr("Invalid debugger's hostname/IP address entered").toStdString().c_str());
		return false;
	}
	if (m_UI->sbOllyPort->value() <= 0 || m_UI->sbOllyPort->value() >= UINT16_MAX)
	{
		info(tr("Invalid debugger's Port entered").toStdString().c_str());
		return false;
	}

	return true;
}

void SettingsDialog::on_twPyPaletteNames_currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)
{
	ScopedEnabler enabler(m_bIgnoreChange);
	Q_UNUSED(enabler);
	updateCurrentPalette();
}

void SettingsDialog::updateCurrentPalette()
{
	QTableWidgetItem* const currentNameItem = m_UI->twPyPaletteNames->currentItem();
	if (!currentNameItem)
		return;
	const PythonPaletteType ppt = static_cast<PythonPaletteType>(currentNameItem->data(PIR_PaletteType).toInt());
	if (ppt != PPT_Dark && ppt != PPT_Light)
		return;
	PythonPalette* const pPalette = static_cast<PythonPalette*>(currentNameItem->data(PIR_PPalette).value<void*>());
	if (!pPalette)
		return;
	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	auto it = pPalette->palette.find(ppet);
	if (it == pPalette->palette.end())
		return;

	const FormatSpec& spec = *it;

	m_UI->wPaletteColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);")
		.arg(spec.color.red())
		.arg(spec.color.green())
		.arg(spec.color.blue())
		.arg(spec.color.alpha()));
	m_UI->wPaletteColor->setProperty(kPropColor.c_str(), QVariant::fromValue(spec.color));
	m_UI->chPaletteBold->setChecked((spec.modifiers & FormatSpec::MOD_Bold) == FormatSpec::MOD_Bold);
	m_UI->chPaletteItalic->setChecked((spec.modifiers & FormatSpec::MOD_Italic) == FormatSpec::MOD_Italic);
	m_UI->fcbFont->setCurrentFont(QFont(pPalette->mainFont, pPalette->mainFontPointSize));
	m_UI->spbFontPointSize->setValue(pPalette->mainFontPointSize);
	m_UI->spbTabWidth->setValue(pPalette->tabWidth);
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_cbPaletteType_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	updateCurrentPalette();
}

PythonPaletteEntryType SettingsDialog::getSelectedPaletteEntryType()
{
	const int currIndex = m_UI->cbPaletteType->currentIndex();
	if (currIndex == -1)
		return PPET_Unknown;
	return static_cast<PythonPaletteEntryType>(m_UI->cbPaletteType->itemData(currIndex, PIR_PaletteType).toInt());
}

PythonPalette* SettingsDialog::getCurrentPalette()
{
	QTableWidgetItem* const currentNameItem = m_UI->twPyPaletteNames->currentItem();
	if (!currentNameItem)
		return nullptr;
	return static_cast<PythonPalette*>(currentNameItem->data(PIR_PPalette).value<void*>());
}

void SettingsDialog::on_bPalettePickColor_clicked()
{
	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	if (PPET_Unknown == ppet)
		return;

	QColor c = m_UI->wPaletteColor->property(kPropColor.c_str()).value<QColor>();
	QColorDialog cd(c);
	cd.setOption(QColorDialog::ShowAlphaChannel);

	if (QColorDialog::Accepted != cd.exec())
		return;

	c = cd.currentColor();

	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;

	pPalette->palette[ppet].color = c;
	m_UI->wPaletteColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);")
		.arg(c.red())
		.arg(c.green())
		.arg(c.blue())
		.arg(c.alpha())
	);
	m_UI->wPaletteColor->setProperty(kPropColor.c_str(), QVariant::fromValue(c));
	m_UI->tePalettePreview->setPalette(*pPalette);
	m_PaletteChanged = true;
}

void SettingsDialog::on_chPaletteBold_clicked()
{
	QCheckBox* cb = qobject_cast<QCheckBox*>(sender());
	if (!cb)
		return;

	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	if (PPET_Unknown == ppet)
		return;
	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;
	if (cb->isChecked())
		pPalette->palette[ppet].modifiers = static_cast<FormatSpec::Modifiers>(pPalette->palette[ppet].modifiers | FormatSpec::MOD_Bold);
	else
		pPalette->palette[ppet].modifiers = static_cast<FormatSpec::Modifiers>(pPalette->palette[ppet].modifiers & ~FormatSpec::MOD_Bold);
	m_PaletteChanged = true;
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_chPaletteItalic_clicked()
{
	QCheckBox* cb = qobject_cast<QCheckBox*>(sender());
	if (!cb)
		return;

	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	if (PPET_Unknown == ppet)
		return;
	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;
	if (cb->isChecked())
		pPalette->palette[ppet].modifiers = static_cast<FormatSpec::Modifiers>(pPalette->palette[ppet].modifiers | FormatSpec::MOD_Italic);
	else
		pPalette->palette[ppet].modifiers = static_cast<FormatSpec::Modifiers>(pPalette->palette[ppet].modifiers & ~FormatSpec::MOD_Italic);
	m_PaletteChanged = true;
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_bResetPalette_clicked()
{
	if (QMessageBox::Yes != QMessageBox::question(nullptr, tr("?"),
			tr("Do you want to reset color schemes configuration to default?"), QMessageBox::Yes, QMessageBox::No))
		return;
	QTableWidgetItem* const itLight = m_UI->twPyPaletteNames->item(0, 0);
	QTableWidgetItem* const itDark = m_UI->twPyPaletteNames->item(1, 0);
	if (!itLight || !itDark)
		return;
	PythonPalette* const lightPalette = static_cast<PythonPalette*>(itLight->data(PIR_PPalette).value<void*>());
	PythonPalette* const darkPalette = static_cast<PythonPalette*>(itDark->data(PIR_PPalette).value<void*>());
	if (!lightPalette || !darkPalette)
		return;
	*lightPalette = PythonPaletteManager::getDefaultLightPalette();
	*darkPalette = PythonPaletteManager::getDefaultDarkPalette();
	m_PaletteChanged = true;

	updateCurrentPalette();
}

void SettingsDialog::on_fcbFont_currentFontChanged(const QFont& fnt)
{
	if (m_bIgnoreChange)
		return;

	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;

	pPalette->mainFont = fnt.family();
	m_PaletteChanged = true;
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_spbFontPointSize_valueChanged(int v)
{
	if (m_bIgnoreChange)
		return;

	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;

	pPalette->mainFontPointSize = v;
	m_PaletteChanged = true;
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_spbTabWidth_valueChanged(int v)
{
	if (m_bIgnoreChange)
		return;

	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;

	pPalette->tabWidth = v;
	m_PaletteChanged = true;
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_bCheckForUpdates_clicked()
{
	util::idapython::github::ReleaseInfo ri;
	std::string error;
	bool ok = false;
	{
		ScopedWaitBox wb("HIDECANCEL\nLabeless: checking for updates...");
		Q_UNUSED(wb);
		ok = util::idapython::github::getLatestRelease(ri, error);
	}
	if (!ok)
	{
		QMessageBox::warning(util::ida::findIDAMainWindow(),
				tr("Error"),
				tr("Unable to get the latest build, error: %1")
					.arg(QString::fromStdString(error)));
		return;
	}

	static const QString currentVer = QString("v_%1").arg(LABELESS_VER_STR).replace(".", "_");
	if (currentVer == ri.tag)
	{
		QMessageBox::information(util::ida::findIDAMainWindow(),
			tr(":)"),
			tr("You are using the latest version"));
		return;
	}

	QMessageBox::information(util::ida::findIDAMainWindow(),
			tr(":)"),
			tr("New release is available:<br><b>ver</b>: %1<br><b>name</b>: %2<br><a href=\"%3\">View on GitHub</a>")
				.arg(ri.tag)
				.arg(ri.name)
				.arg(ri.url));
}

void SettingsDialog::on_chFuncLocalVarsAll_toggled(bool v)
{
	m_UI->chFuncLocalVars->setEnabled(!v);
}

void SettingsDialog::on_bgAutoCompletion_toggled(bool v)
{
	if (v)
		QMessageBox::information(this, tr("Note!"), tr("To turn on auto-completion restart is required"));
}

bool SettingsDialog::getLightPalette(PythonPalette& result) const
{
	if (QTableWidgetItem* const itLight = m_UI->twPyPaletteNames->item(0, 0))
	{
		if (PythonPalette* const lightPalette = static_cast<PythonPalette*>(itLight->data(PIR_PPalette).value<void*>()))
		{
			result = *lightPalette;
			return true;
		}
	}
	return false;
}

bool SettingsDialog::getDarkPalette(PythonPalette& result) const
{
	if (QTableWidgetItem* const itDark = m_UI->twPyPaletteNames->item(1, 0))
	{
		if (PythonPalette* const darkPalette = static_cast<PythonPalette*>(itDark->data(PIR_PPalette).value<void*>()))
		{
			result = *darkPalette;
			return true;
		}
	}
	return false;
}
