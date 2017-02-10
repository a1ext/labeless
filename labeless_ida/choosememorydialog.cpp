/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "choosememorydialog.h"
#include "ui_choosememorydialog.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#   include <QtWidgets/QMessageBox>
#else // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#   include <QMessageBox>
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

#include <QButtonGroup>
#include <QMenu>

#include "util/util_ida.h"

namespace {

QString ollyStyleFormatHex(ea_t v)
{
	return QString("%1").arg(v, sizeof(ea_t) * 2, 16, QChar('0')).replace("0x", "").toUpper();
}

enum
{
	COL_BASE,
	COL_SIZE,
	COL_OWNER,
	COL_PROTECT
};

enum RB_Manual
{
	RBM_VA_To,
	RBM_Size
};

static const QString kRedBorderStyleSheet = "border: 3px solid red;";


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

	QButtonGroup* bg = new QButtonGroup(this);
	bg->addButton(m_UI->rbVaTo, RBM_VA_To);
	bg->addButton(m_UI->rbSize, RBM_Size);
	bg->setExclusive(true);
	CHECKED_CONNECT(connect(bg, SIGNAL(buttonClicked(int)), this, SLOT(onManualMeasureTypeChanged(int))));
	m_UI->gbManual->setChecked(true);
	m_UI->rbSize->click();
	m_UI->gbManual->setChecked(false);

	fillView();
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
		m_UI->leManualVaFrom->text().toULongLong(nullptr, 16),
		m_UI->leManualSize->text().toULongLong(nullptr, 16),
		PAGE_EXECUTE_READWRITE,
		true);
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
		bool ok1, ok2, ok3;
		const qulonglong base = m_UI->leManualVaFrom->text().toULongLong(&ok1, 16);
		const qulonglong size = m_UI->leManualSize->text().toULongLong(&ok2, 16);
		const qulonglong eaEnd = m_UI->leManualVaTo->text().toULongLong(&ok3, 16);
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

		if (base >= eaEnd)
		{
			QMessageBox::warning(this, tr(":("), tr("Empty or invalid memory range is specified"));
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

void ChooseMemoryDialog::on_twMemoryMap_customContextMenuRequested(const QPoint&)
{
	QTableWidget* tw = qobject_cast<QTableWidget*>(sender());
	if (!tw)
		return;

	auto items = tw->selectionModel()->selectedRows();
	QMenu m(this);
	QAction* actMarkAsRWE = m.addAction(tr("Import as RWE"));
	QAction* rv = m.exec(QCursor::pos());
	if (!rv || rv != actMarkAsRWE)
		return;

	for (int i = 0; i < items.count(); ++i)
	{
		const auto idx = items.at(i);
		if (!idx.isValid())
			continue;
		const auto row = idx.row();
		auto& r = m_MemMap[row];
		r.protect = PAGE_EXECUTE_READWRITE;
		r.forceProtect = true;

		m_UI->twMemoryMap->item(row, COL_PROTECT)->setText(util::ida::memoryProtectToStr(m_MemMap[row].protect));
	}
}

void ChooseMemoryDialog::on_leManualVaFrom_textChanged(const QString& v)
{
	Q_UNUSED(v);
	QLineEdit* le = qobject_cast<QLineEdit*>(sender());
	if (!le)
		return;

	bool fromOk = false;
	const qulonglong from = le->text().toULongLong(&fromOk, 16);

	if (!m_UI->leManualVaTo->text().isEmpty() || !m_UI->leManualSize->text().isEmpty())
	{
		QLineEdit* lePrimary = m_UI->rbSize->isChecked() ? m_UI->leManualSize : m_UI->leManualVaTo;
		QLineEdit* leSecondary = m_UI->rbSize->isChecked() ? m_UI->leManualVaTo : m_UI->leManualSize;

		ScopedSignalBlocker signalBlocker(QList<QPointer<QObject>>() << leSecondary);
		Q_UNUSED(signalBlocker);

		bool ok = false;
		uval_t size = static_cast<uval_t>(lePrimary->text().toULongLong(&ok, 16));
		if (!ok || !size)
			return;
		leSecondary->setText(ollyStyleFormatHex(from + size));
		leSecondary->setStyleSheet(QString::null);
		lePrimary->setStyleSheet(QString::null);
	}

	le->setStyleSheet(!fromOk || from > BADADDR ? kRedBorderStyleSheet : QString::null);
}

void ChooseMemoryDialog::on_leManualVaTo_textChanged(const QString& v)
{
	Q_UNUSED(v);
	QLineEdit* le = qobject_cast<QLineEdit*>(sender());
	if (!le)
		return;

	bool endOk = false;
	const qulonglong ea_end = le->text().toULongLong(&endOk, 16);
	if (m_UI->leManualVaFrom->text().isEmpty())
		return;

	ScopedSignalBlocker signalBlocker(QList<QPointer<QObject>>() << m_UI->leManualSize);
	Q_UNUSED(signalBlocker);

	if (!endOk)
	{
		m_UI->leManualSize->clear();
		le->setStyleSheet(kRedBorderStyleSheet);
		return;
	}
	bool ok = false;
	const qulonglong from = m_UI->leManualVaFrom->text().toULongLong(&ok, 16);
	if (!ok || !from)
		return;

	le->setStyleSheet(from >= ea_end || ea_end > BADADDR ? kRedBorderStyleSheet : QString::null);
	m_UI->leManualSize->setText(ollyStyleFormatHex(ea_end - from));
	m_UI->leManualSize->setStyleSheet(QString::null);
}

void ChooseMemoryDialog::on_leManualSize_textChanged(const QString& v)
{
	Q_UNUSED(v);
	QLineEdit* le = qobject_cast<QLineEdit*>(sender());
	if (!le)
		return;

	bool sizeOk = false;
	const qulonglong size = le->text().toULongLong(&sizeOk, 16);
	if (m_UI->leManualVaFrom->text().isEmpty())
		return;

	ScopedSignalBlocker signalBlocker(QList<QPointer<QObject>>() << m_UI->leManualVaTo);
	Q_UNUSED(signalBlocker);

	if (!sizeOk)
	{
		m_UI->leManualVaTo->clear();
		le->setStyleSheet(kRedBorderStyleSheet);
		return;
	}

	bool ok = false;
	const qulonglong from = m_UI->leManualVaFrom->text().toULongLong(&ok, 16);
	if (!ok || !from)
		return;

	le->setStyleSheet(size > BADADDR || from + size > BADADDR ? kRedBorderStyleSheet : QString::null);
	m_UI->leManualVaTo->setText(ollyStyleFormatHex(from + size));
	m_UI->leManualVaTo->setStyleSheet(QString::null);
}

void ChooseMemoryDialog::onManualMeasureTypeChanged(int t)
{
	auto v = static_cast<RB_Manual>(t);

	m_UI->leManualSize->setEnabled(v == RBM_Size);
	m_UI->leManualVaTo->setEnabled(v == RBM_VA_To);
	(v == RBM_Size ? m_UI->leManualSize : m_UI->leManualVaTo)->setFocus();
}

void ChooseMemoryDialog::fillView()
{
	for (auto it = m_MemMap.constBegin(), end = m_MemMap.constEnd(); it != end; ++it)
	{
		const MemoryRegion& mr = *it;
		const int row = m_UI->twMemoryMap->rowCount();
		m_UI->twMemoryMap->insertRow(row);
		m_UI->twMemoryMap->setItem(row, COL_BASE, new QTableWidgetItem(ollyStyleFormatHex(mr.base)));
		m_UI->twMemoryMap->setItem(row, COL_SIZE, new QTableWidgetItem(ollyStyleFormatHex(mr.size)));
		m_UI->twMemoryMap->setItem(row, COL_OWNER, new QTableWidgetItem(QString::fromStdString(mr.name)));
		m_UI->twMemoryMap->setItem(row, COL_PROTECT, new QTableWidgetItem(util::ida::memoryProtectToStr(mr.protect)));
	}
	m_UI->twMemoryMap->horizontalHeader()->setStretchLastSection(true);
	m_UI->twMemoryMap->adjustSize();
}

bool ChooseMemoryDialog::isRangeBelongsToExistingRegions(uint64_t base, uint64_t size) const
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
