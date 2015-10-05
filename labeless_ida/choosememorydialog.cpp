/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "choosememorydialog.h"
#include "ui_choosememorydialog.h"

#include <QMessageBox>

#include "hlp.h"

namespace {

QString ollyStyleFormatHex(DWORD_PTR v)
{
	return QString("%1").arg(v, 8, 16, QChar('0')).replace("0x", "").toUpper();
}

} // anonymous

ChooseMemoryDialog::ChooseMemoryDialog(const MemoryRegionList& memMap, const QString& title, QWidget* parent)
	: QDialog(parent)
	, m_UI(new Ui::ChooseMemoryDialog)
	, m_MemMap(memMap)
{
	m_UI->setupUi(this);
	if (!title.isEmpty())
		setWindowTitle(title);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	fillView();
	m_UI->gbManual->setChecked(false);
}

ChooseMemoryDialog::~ChooseMemoryDialog()
{
	delete m_UI;
}

bool ChooseMemoryDialog::getSelectedMemory(MemoryRegionList& selected) const
{
	selected.clear();
	if (m_UI->gbRegions->isChecked())
	{
		auto items = m_UI->twMemoryMap->selectionModel()->selectedRows();
		for (int i = 0; i < items.count(); ++i)
		{
			const auto idx = items.at(i);
			if (!idx.isValid())
				continue;
			const auto row = idx.row();
			selected.append(m_MemMap.at(row));
		}
		return true;
	}
	MemoryRegion r(
		m_UI->leManualVaFrom->text().toInt(nullptr, 16),
		m_UI->leManualSize->text().toUInt(nullptr, 16),
		0);
	selected.append(r);
	return true;
}

void ChooseMemoryDialog::changeEvent(QEvent* e)
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

void ChooseMemoryDialog::accept()
{
	if (m_UI->gbManual->isChecked())
	{
		bool ok1, ok2;
		const ea_t base = m_UI->leManualVaFrom->text().toUInt(&ok1, 16);
		const uint32 size = m_UI->leManualSize->text().toUInt(&ok2, 16);
		if (!ok1 || !base)
		{
			QMessageBox::warning(this, tr(":("), tr("<b>'VA from'</b> field is invalid"));
			m_UI->leManualVaFrom->setFocus();
			return;
		}
		if (!ok2 || !size)
		{
			QMessageBox::warning(this, tr(":("), tr("<b>'Size'</b> field is invalid"));
			m_UI->leManualSize->setFocus();
			return;
		}
		if (!isRangeBelongsToExistingRegions(base, size))
		{
			QMessageBox::warning(this, tr(":("),
				tr("Memory range [%1, %2) specified manually doesn't belongs to any existing memory region")
				.arg(ollyStyleFormatHex(base))
				.arg(ollyStyleFormatHex(base + size)));
			return;
		}
		QDialog::accept();
		return;
	}
	if (m_UI->twMemoryMap->selectedItems().isEmpty())
	{
		QMessageBox::warning(this, tr(":("), tr("You should select memory regions to dump or put <b>VA base</b> and <b>Size</b> manually"));
		return;
	}
	QDialog::accept();
}

void ChooseMemoryDialog::selectTypeChanged()
{
	auto sndr = qobject_cast<QGroupBox*>(sender());
	const bool isCheck = !sndr->isChecked();

	QGroupBox* const targetToUncheck = qobject_cast<QGroupBox*>(sender()) == m_UI->gbManual
		? m_UI->gbRegions
		: m_UI->gbManual;
	targetToUncheck->setChecked(isCheck);
	if (m_UI->gbManual->isChecked())
		m_UI->leManualVaFrom->setFocus();
}

void ChooseMemoryDialog::fillView()
{
	enum
	{
		COL_BASE,
		COL_SIZE,
		COL_OWNER,
		COL_PROTECT
	};

	for (auto it = m_MemMap.constBegin(), end = m_MemMap.constEnd(); it != end; ++it)
	{
		const MemoryRegion& mr = *it;
		const int row = m_UI->twMemoryMap->rowCount();
		m_UI->twMemoryMap->insertRow(row);
		m_UI->twMemoryMap->setItem(row, COL_BASE, new QTableWidgetItem(ollyStyleFormatHex(mr.base)));
		m_UI->twMemoryMap->setItem(row, COL_SIZE, new QTableWidgetItem(ollyStyleFormatHex(mr.size)));
		m_UI->twMemoryMap->setItem(row, COL_OWNER, new QTableWidgetItem(QString::fromStdString(mr.name)));
		m_UI->twMemoryMap->setItem(row, COL_PROTECT, new QTableWidgetItem(QString::fromStdString(hlp::memoryProtectToStr(mr.protect))));
	}
	m_UI->twMemoryMap->horizontalHeader()->setStretchLastSection(true);
	m_UI->twMemoryMap->adjustSize();
}

bool ChooseMemoryDialog::isRangeBelongsToExistingRegions(ea_t base, uint32 size) const
{
	MemoryRegion rmIn(base, size, 0);
	for (int i = 0; i < m_MemMap.size(); ++i)
	{
		const auto& mr = m_MemMap.at(i);
		if (mr.isIntersects(rmIn))
			return true;
	}
	return false;
}
