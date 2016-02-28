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
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTextCharFormat>

#include "types.h"
#include "globalsettingsmanager.h"
#include "pythonpalettemanager.h"
#include "../common/version.h"

namespace {

static const std::string kPropColor = "p_color";

} // anonymous

SettingsDialog::SettingsDialog(const Settings& settings, qulonglong currModBase, QWidget* parent)
	: QDialog(parent)
	, m_UI(new Ui::SettingsDialog)
	, m_PaletteChanged(false)
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
	m_UI->leRemoteModuleBase->setText(QString("0x%1").arg(settings.remoteModBase, 8, 16, QChar('0')));
	m_UI->leRemoteModuleBase->setToolTip(QString("Current IDA DB's module base is 0x%1.").arg(currModBase, 8, 16, QChar('0')));
	m_UI->gbEnabledSync->setChecked(settings.enabled);
	m_UI->chDemangleNames->setChecked(settings.demangle);
	m_UI->chLocalLabels->setChecked(settings.localLabels);
	m_UI->chPerformPEAnalysis->setChecked(settings.analysePEHeader);
	m_UI->chPostProcessFixCallJumps->setChecked(settings.postProcessFixCallJumps);
	m_UI->leExternSegDefSize->setText(QString("0x%1").arg(settings.defaultExternSegSize, 8, 16, QChar('0')).toUpper());
	m_UI->chNonCodeNames->setChecked(settings.nonCodeNames);
	m_UI->cbOverwriteWarning->setCurrentIndex(settings.overwriteWarning);
	m_UI->cbCommentsSync->setCurrentIndex(settings.commentsSync);

	QLabel* const lVer = new QLabel(m_UI->tabWidget);
	lVer->setText(QString("v %1").arg(LABELESS_VER_STR));
	lVer->setStyleSheet("color: rgb(0x8a, 0x8a, 0x8a);");
	QGraphicsDropShadowEffect* const effect = new QGraphicsDropShadowEffect(lVer);
	effect->setBlurRadius(5);
	effect->setOffset(0);
	effect->setColor(Qt::white);
	lVer->setGraphicsEffect(effect);
	m_UI->tabWidget->setCornerWidget(lVer);

	setUpPalette();
	setFixedSize(size());
	adjustSize();
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

SettingsDialog::~SettingsDialog()
{
	QTableWidgetItem* const itLight = m_UI->twPyPaletteNames->item(0, 0);
	QTableWidgetItem* const itDark = m_UI->twPyPaletteNames->item(1, 0);
	if (itLight)
	{
		if (PythonPalette* const lightPalette = static_cast<PythonPalette*>(itLight->data(Qt::UserRole + 1).value<void*>()))
			delete lightPalette;
	}
	if (!itDark)
	{
		if (PythonPalette* const darkPalette = static_cast<PythonPalette*>(itLight->data(Qt::UserRole + 1).value<void*>()))
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
		{ tr("Number"),											PPET_Number }
	};

	QTableWidgetItem* const light = new QTableWidgetItem(tr("light"));
	light->setData(Qt::UserRole, PPT_Light);
	auto pLightPalette = new PythonPalette(PythonPaletteManager::instance().lightPalette());
	light->setData(Qt::UserRole + 1, QVariant::fromValue<void*>(pLightPalette));

	QTableWidgetItem* const dark = new QTableWidgetItem(tr("dark"));
	dark->setData(Qt::UserRole, PPT_Dark);
	auto pDarkPalette = new PythonPalette(PythonPaletteManager::instance().darkPalette());
	dark->setData(Qt::UserRole + 1, QVariant::fromValue<void*>(pDarkPalette));

	for (unsigned i = 0; i < _countof(kPaletteEntryTypes); ++i)
		m_UI->cbPaletteType->addItem(kPaletteEntryTypes[i].name, kPaletteEntryTypes[i].type);

	m_UI->twPyPaletteNames->setRowCount(2);
	m_UI->twPyPaletteNames->setItem(0, 0, light);
	m_UI->twPyPaletteNames->setItem(1, 0, dark);

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
	
	result.enabled = m_UI->gbEnabledSync->isChecked();
	result.demangle = m_UI->chDemangleNames->isChecked();
	result.localLabels = m_UI->chLocalLabels->isChecked();
	result.nonCodeNames = m_UI->chNonCodeNames->isChecked();
	result.analysePEHeader = m_UI->chPerformPEAnalysis->isChecked();
	result.defaultExternSegSize = m_UI->leExternSegDefSize->text().toULongLong(nullptr, 16);
	result.postProcessFixCallJumps = m_UI->chPostProcessFixCallJumps->isChecked();
	result.overwriteWarning = static_cast<Settings::OverwriteWarning>(m_UI->cbOverwriteWarning->currentIndex());
	result.commentsSync = static_cast<Settings::CommentsSync>(m_UI->cbCommentsSync->currentIndex());
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
		info(tr("Invalid Olly's hostname/IP address entered").toStdString().c_str());
		return false;
	}
	if (m_UI->sbOllyPort->value() <= 0 || m_UI->sbOllyPort->value() >= UINT16_MAX)
	{
		info(tr("Invalid Olly's Port entered").toStdString().c_str());
		return false;
	}
	bool ok = false;
	const uint32_t defaultExternSegSize = m_UI->leExternSegDefSize->text().toUInt(&ok, 16);
	if (!ok || defaultExternSegSize < 0x1000 || (defaultExternSegSize % sizeof(DWORD_PTR)))
	{
		info(tr("Size of \"extern\" segment in hex is <b>invalid</b>.<br>Should be multiple by DWORD_PTR size(for i686 arch = 4) and not less than 0x1000.").toStdString().c_str());
		return false;
	}

	return true;
}

void SettingsDialog::on_twPyPaletteNames_currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)
{
	updateCurrentPalette();
}

void SettingsDialog::updateCurrentPalette()
{
	QTableWidgetItem* const currentNameItem = m_UI->twPyPaletteNames->currentItem();
	if (!currentNameItem)
		return;
	const PythonPaletteType ppt = static_cast<PythonPaletteType>(currentNameItem->data(Qt::UserRole).toInt());
	if (ppt != PPT_Dark && ppt != PPT_Light)
		return;
	PythonPalette* const pPalette = static_cast<PythonPalette*>(currentNameItem->data(Qt::UserRole + 1).value<void*>());
	if (!pPalette)
		return;
	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	auto it = pPalette->palette.find(ppet);
	if (it == pPalette->palette.end())
		return;

	const FormatSpec& spec = *it;

	m_UI->wPaletteColor->setStyleSheet(QString("background-color: %1;").arg(spec.color.name()));
	m_UI->wPaletteColor->setProperty(kPropColor.c_str(), QVariant::fromValue(spec.color));
	m_UI->chPaletteBold->setChecked((spec.modifiers & FormatSpec::MOD_Bold) == FormatSpec::MOD_Bold);
	m_UI->chPaletteItalic->setChecked((spec.modifiers & FormatSpec::MOD_Italic) == FormatSpec::MOD_Italic);
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_cbPaletteType_currentIndexChanged(int index)
{
	updateCurrentPalette();
}

PythonPaletteEntryType SettingsDialog::getSelectedPaletteEntryType()
{
	const int currIndex = m_UI->cbPaletteType->currentIndex();
	if (currIndex == -1)
		return PPET_Unknown;
	return static_cast<PythonPaletteEntryType>(m_UI->cbPaletteType->itemData(currIndex, Qt::UserRole).toInt());
}

PythonPalette* SettingsDialog::getCurrentPalette()
{
	QTableWidgetItem* const currentNameItem = m_UI->twPyPaletteNames->currentItem();
	if (!currentNameItem)
		return nullptr;
	return static_cast<PythonPalette*>(currentNameItem->data(Qt::UserRole + 1).value<void*>());
}

void SettingsDialog::on_bPalettePickColor_clicked()
{
	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	if (PPET_Unknown == ppet)
		return;

	QColor c = m_UI->wPaletteColor->property(kPropColor.c_str()).value<QColor>();
	QColorDialog cd(c);
	if (QColorDialog::Accepted != cd.exec())
		return;

	c = cd.currentColor();

	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;

	pPalette->palette[ppet].color = c;
	m_UI->wPaletteColor->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	m_UI->wPaletteColor->setProperty(kPropColor.c_str(), QVariant::fromValue(c));
	m_UI->tePalettePreview->setPalette(*pPalette);
	m_PaletteChanged = true;
}

void SettingsDialog::on_chPaletteBold_toggled(bool value)
{
	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	if (PPET_Unknown == ppet)
		return;
	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;
	if (value)
		pPalette->palette[ppet].modifiers = static_cast<FormatSpec::Modifiers>(pPalette->palette[ppet].modifiers | FormatSpec::MOD_Bold);
	else
		pPalette->palette[ppet].modifiers = static_cast<FormatSpec::Modifiers>(pPalette->palette[ppet].modifiers & ~FormatSpec::MOD_Bold);
	m_PaletteChanged = true;
	m_UI->tePalettePreview->setPalette(*pPalette);
}

void SettingsDialog::on_chPaletteItalic_toggled(bool value)
{
	const PythonPaletteEntryType ppet = getSelectedPaletteEntryType();
	if (PPET_Unknown == ppet)
		return;
	PythonPalette* const pPalette = getCurrentPalette();
	if (!pPalette)
		return;
	if (value)
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
	PythonPalette* const lightPalette = static_cast<PythonPalette*>(itLight->data(Qt::UserRole + 1).value<void*>());
	PythonPalette* const darkPalette = static_cast<PythonPalette*>(itLight->data(Qt::UserRole + 1).value<void*>());
	if (!lightPalette || !darkPalette)
		return;
	*lightPalette = PythonPaletteManager::getDefaultLightPalette();
	*darkPalette = PythonPaletteManager::getDefaultDarkPalette();
	m_PaletteChanged = true;

	updateCurrentPalette();
}

bool SettingsDialog::getLightPalette(PythonPalette& result) const
{
	if (QTableWidgetItem* const itLight = m_UI->twPyPaletteNames->item(0, 0))
	{
		if (PythonPalette* const lightPalette = static_cast<PythonPalette*>(itLight->data(Qt::UserRole + 1).value<void*>()))
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
		if (PythonPalette* const darkPalette = static_cast<PythonPalette*>(itDark->data(Qt::UserRole + 1).value<void*>()))
		{
			result = *darkPalette;
			return true;
		}
	}
	return false;
}
