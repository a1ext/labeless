/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#include "pyollyview.h"
#include "ui_pyollyview.h"
#include "types.h"
#include "pythonpalettemanager.h"

#include <QEvent>
#include <QFontMetrics>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>
#include "globalsettingsmanager.h"

namespace {

static const QString kColorSchemeLight = "light";
static const QString kColorSchemeDark = "dark";
//static const QString kTemplates = "templates";

static const QString kFastActionN = "fast_action_%1";
static const QString kFastActionIDAScript = "ida";
static const QString kFastActionOllyScript = "olly";

static const QString kIDA = "ida";
static const QString kOlly = "olly";
static const QString kUnknown = "unknown";

static const std::string kPropName = "p_name";
static const std::string kPropText = "p_text";
static const std::string kPropType = "p_type";
static const std::string kPropNumber = "p_num";

enum StoredTemplateType
{
	STT_IDA,
	STT_Olly
};

inline QString storedTemplateTypeToString(StoredTemplateType type)
{
	switch (type)
	{
	case STT_IDA:
		return kIDA;
	case STT_Olly:
		return kOlly;
	default:
		return kUnknown;
	}
}

QMap<QString, QString> getStoredTemplates(StoredTemplateType type)
{
	QMap<QString, QString> rv;

	const QVariantMap vm = GlobalSettingsManger::instance().
			value(GSK_Templates, QVariant(), storedTemplateTypeToString(type)).toMap();

	for (auto it = vm.constBegin(), end = vm.constEnd(); it != end; ++it)
		rv.insert(it.key(), it.value().toString());

	return rv;
}

void storeTemplates(const QMap<QString, QString>& templates, StoredTemplateType type)
{
	QVariantMap vm;
	for (auto it = templates.constBegin(), end = templates.constEnd(); it != end; ++it)
		vm.insert(it.key(), it.value());

	GlobalSettingsManger::instance().
			setValue(GSK_Templates, vm, storedTemplateTypeToString(type));
}

} // anonymous

PyOllyView::PyOllyView(bool isShowAllResponsesInLog, QWidget* parent)
	: QWidget(parent)
	, m_UI(new Ui::PyOllyView)
{
	m_UI->setupUi(this);
	m_UI->chShowAllResponsesInLog->setChecked(isShowAllResponsesInLog);

	setUpGUI();
	setUpConnections();
}

PyOllyView::~PyOllyView()
{
	delete m_UI;
}

void PyOllyView::setUpGUI()
{
	const QString scheme = GlobalSettingsManger::instance().
			value(GSK_ColorScheme, kColorSchemeLight).toString().toLower();

	m_UI->teIDAScript->setPalette(PythonPaletteManager::instance().palette());
	m_UI->teOllyScript->setPalette(PythonPaletteManager::instance().palette());

	int idx = m_UI->cbColorScheme->findText(scheme, static_cast<Qt::MatchFlags>(Qt::MatchExactly));
	m_UI->cbColorScheme->setCurrentIndex(idx);

	m_UI->horSplitter->setStretchFactor(0, 1);
	m_UI->horSplitter->setStretchFactor(1, 2);

	m_UI->vertSplitter->setStretchFactor(0, 3);
	m_UI->vertSplitter->setStretchFactor(1, 1);

	QFont f;
	f.setFamily("Courier");
	f.setFixedPitch(true);
	f.setPointSize(9);
	const int w = QFontMetrics(f).width(' ') * 4;
	m_UI->teLog->setFont(f);
	m_UI->teLog->setTabStopWidth(w);
	m_UI->teLogErr->setFont(f);
	m_UI->teLogErr->setTabStopWidth(w);

	m_UI->teLog->viewport()->installEventFilter(this);
	m_UI->teLogErr->viewport()->installEventFilter(this);

	for (int i = 0; i < 4; ++i)
	{
		QPushButton* pb = findChild<QPushButton*>(QString("bFastAction%1").arg(i+1));
		if (!pb)
			continue;
		const int num = pb->property(kPropNumber.c_str()).toInt();

		QVariantMap vm;
		do {
			auto ss = GlobalSettingsManger::instance().get();
			vm = ss->settings->value(kFastActionN.arg(num)).toMap();
		} while (0);

		if (vm.contains(kFastActionIDAScript) && vm.contains(kFastActionOllyScript))
		{
			QFont fnt = pb->font();
			fnt.setBold(true);
			pb->setFont(fnt);
		}

		QAction* const aBind = new QAction(tr("Bind current IDA && Olly scripts to this button"), pb);
		aBind->setProperty(kPropNumber.c_str(), num);
		CHECKED_CONNECT(connect(aBind, SIGNAL(triggered()), this, SLOT(onBindScriptsToFastActionRequested())));
		pb->addAction(aBind);

		QAction* const aClearBinding = new QAction(tr("Clear binding"), pb);
		aClearBinding->setProperty(kPropNumber.c_str(), num);
		CHECKED_CONNECT(connect(aClearBinding, SIGNAL(triggered()), this, SLOT(onClearBindingScriptsOfFastAction())));
		pb->addAction(aClearBinding);

		CHECKED_CONNECT(connect(pb, SIGNAL(clicked()), this, SLOT(onFastActionRequested())));
	}
}

void PyOllyView::setUpConnections()
{
	CHECKED_CONNECT(connect(m_UI->chShowAllResponsesInLog, SIGNAL(toggled(bool)),
		this, SIGNAL(showAllResponsesInLogToggled(bool))));
	CHECKED_CONNECT(connect(m_UI->bRunScript, SIGNAL(clicked()),
		this, SIGNAL(runScriptRequested())));
	CHECKED_CONNECT(connect(m_UI->bSettings, SIGNAL(clicked()),
		this, SIGNAL(settingsRequested())));
	CHECKED_CONNECT(connect(m_UI->cbColorScheme, SIGNAL(currentIndexChanged(int)),
		this, SLOT(onColorSchemeChanged())));
	CHECKED_CONNECT(connect(m_UI->bClearLog, SIGNAL(clicked()),
		this, SIGNAL(clearLogsRequested())));
	CHECKED_CONNECT(connect(m_UI->teIDAScript, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(onScriptEditContextMenuRequested(const QPoint&))));
	CHECKED_CONNECT(connect(m_UI->teOllyScript, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(onScriptEditContextMenuRequested(const QPoint&))));
}

void PyOllyView::changeEvent(QEvent* e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		m_UI->retranslateUi(this);
		break;
	default:
		break;
	}
}

void PyOllyView::enableRunScriptButton(bool enabled)
{
	m_UI->bRunScript->setEnabled(enabled);
}

void PyOllyView::prependStdoutLog(const QString& text, bool isHtml)
{
	m_UI->teLog->moveCursor(QTextCursor::Start);
	if (isHtml)
		m_UI->teLog->insertHtml(text);
	else
		m_UI->teLog->insertPlainText(text);
	m_UI->teLog->moveCursor(QTextCursor::Start);
}

void PyOllyView::prependStderrLog(const QString& text, bool isHtml)
{
	m_UI->teLogErr->moveCursor(QTextCursor::Start);
	if (isHtml)
		m_UI->teLogErr->insertHtml(text);
	else
		m_UI->teLogErr->insertPlainText(text);
	m_UI->teLogErr->moveCursor(QTextCursor::Start);
}

QString PyOllyView::getOllyScript(bool html) const
{
	QString result;
	if (html && m_UI->teOllyScript->asHighlightedHtml(result))
		return result;
	return m_UI->teOllyScript->toPlainText();
}

QString PyOllyView::getIDAScript(bool html) const
{
	QString result;
	if (html && m_UI->teIDAScript->asHighlightedHtml(result))
		return result;
	return m_UI->teIDAScript->toPlainText();
}

void PyOllyView::onColorSchemeChanged()
{
	const int index = m_UI->cbColorScheme->currentIndex();
	if (index == -1)
		return;
	const QString sel = m_UI->cbColorScheme->itemText(index).toLower();
	const bool isDark = sel == "dark";

	PythonPaletteManager& palette = PythonPaletteManager::instance();
	palette.switchScheme(isDark);
	m_UI->teIDAScript->colorSchemeChanged();
	m_UI->teOllyScript->colorSchemeChanged();

	GlobalSettingsManger::instance().setValue(GSK_ColorScheme, sel);
}

bool PyOllyView::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() != QEvent::MouseButtonPress)
		return QObject::eventFilter(obj, event);

	QTextEdit* const textEdit = qobject_cast<QTextEdit*>(obj->parent());
	if (!textEdit || (textEdit != m_UI->teLog && textEdit != m_UI->teLogErr))
		return QObject::eventFilter(obj, event);

	QMouseEvent* const me = static_cast<QMouseEvent*>(event);
	const QString anchor = textEdit->anchorAt(me->pos());
	if (!anchor.isEmpty())
	{
		emit anchorClicked(anchor);
		return true;
	}
	return QObject::eventFilter(obj, event);
}

void PyOllyView::setOllyScript(const QString& text)
{
	m_UI->teOllyScript->setText(text);
}

void PyOllyView::setIDAScript(const QString& text)
{
	m_UI->teIDAScript->setText(text);
}

void PyOllyView::onScriptEditContextMenuRequested(const QPoint& pt)
{
	TextEdit* const te = qobject_cast<TextEdit*>(sender());
	if (!te)
		return;

	QMenu* const menu = te->createStandardContextMenu();
	menu->addSeparator();
	QMenu* const m = menu->addMenu(tr("Templates"));

	QSet<QAction*> knownActions;
	QSet<QAction*> removeActions;

	auto actionSave = m->addAction(tr("Save as template..."));
	knownActions << actionSave;

	const bool isOlly = te == m_UI->teOllyScript;
	const StoredTemplateType type = isOlly ? STT_Olly : STT_IDA;

	QMap<QString, QString>& templates = type == STT_IDA ? m_IDATemplates : m_OllyTemplates;
	if (templates.isEmpty())
		templates = getStoredTemplates(type); // Invalidate


	for (auto it = templates.constBegin(), end = templates.constEnd(); it != end; ++it)
	{
		QMenu* const tmenu = m->addMenu(it.key());
		QAction* const loadAction = tmenu->addAction(tr("Load"), this, SLOT(onLoadTemplateRequested()));
		loadAction->setProperty(kPropText.c_str(), it.value());
		loadAction->setProperty(kPropType.c_str(), static_cast<int>(type));
		QAction* const removeAction = tmenu->addAction(tr("Remove"));
		removeAction->setProperty(kPropName.c_str(), it.key());
		removeActions << removeAction;
		knownActions << removeAction;
	}

	QAction* const selected = menu->exec(te->mapToGlobal(pt));
	menu->deleteLater();
	if (!selected || !knownActions.contains(selected))
		return;

	if (selected == actionSave)
	{
		QString name;

		while (true)
		{
			name = QInputDialog::getText(nullptr, tr("New template"), tr("Template name"), QLineEdit::Normal, name);
			if (name.isEmpty())
				return;

			if (!templates.contains(name))
				break;
			const int sel = askyn_c(ASKBTN_NO,
				"The template with name '%s' is already saved. Do you want to update it?",
				name.toStdString().c_str());
			if (sel == ASKBTN_YES)
				break;
			if (sel == ASKBTN_CANCEL)
				return;
		}

		templates[name] = te->toPlainText();
		storeTemplates(templates, type);
		return;
	}

	if (removeActions.contains(selected))
	{
		const QString name = selected->property(kPropName.c_str()).toString();
		if (!templates.contains(name))
			return;
		templates.remove(name);
		storeTemplates(templates, type);
		return;
	}
}

void PyOllyView::onLoadTemplateRequested()
{
	QAction* const action = qobject_cast<QAction*>(sender());
	if (!action)
		return;
	const StoredTemplateType type = static_cast<StoredTemplateType>(action->property(kPropType.c_str()).toInt());
	const QString value = action->property(kPropText.c_str()).toString();
	
	TextEdit* const te = type == STT_IDA
		? m_UI->teIDAScript
		: m_UI->teOllyScript;
	te->setText(value);
}

void PyOllyView::onFastActionRequested()
{
	QPushButton* const btn = qobject_cast<QPushButton*>(sender());
	if (!btn)
		return;

	const int number = btn->property(kPropNumber.c_str()).toInt();

	QVariantMap vm;
	do {
		auto ss = GlobalSettingsManger::instance().get();
		vm =ss->settings->value(kFastActionN.arg(number)).toMap();
	} while (0);


	if (!vm.contains(kFastActionIDAScript) || !vm.contains(kFastActionOllyScript))
	{
		info("No scripts are binded to that control. Use right click to assign a script.");
		return;
	}

	const auto ollyScript = vm[kFastActionOllyScript].toString();
	const auto idaScript = vm[kFastActionIDAScript].toString();

	if (((!m_UI->teIDAScript->toPlainText().isEmpty() && m_UI->teIDAScript->toPlainText() != idaScript) ||
		(!m_UI->teOllyScript->toPlainText().isEmpty() && m_UI->teOllyScript->toPlainText() != ollyScript)))
	{
		if (ASKBTN_YES != askyn_c(ASKBTN_YES, "Do you want to execute stored FastAction (current scripts will be lost)?"))
			return;
	}
	m_UI->teIDAScript->setText(idaScript);
	m_UI->teOllyScript->setText(ollyScript);
	emit runScriptRequested();
}

void PyOllyView::onBindScriptsToFastActionRequested()
{
	QAction* const action = qobject_cast<QAction*>(sender());
	if (!action)
		return;
	const int num = action->property(kPropNumber.c_str()).toInt();

	const auto ollyScript = m_UI->teOllyScript->toPlainText();
	const auto idaScript = m_UI->teIDAScript->toPlainText();

	if (ollyScript.isEmpty() && idaScript.isEmpty())
	{
		info("Fill in scripts before binding to FastAction!");
		return;
	}

	QVariantMap vm;
	vm[kFastActionIDAScript] = idaScript;
	vm[kFastActionOllyScript] = ollyScript;

	do {
		auto ss = GlobalSettingsManger::instance().get();
		ss->settings->setValue(kFastActionN.arg(num), vm);
	} while (0);

	if (QPushButton* pb = findChild<QPushButton*>(QString("bFastAction%1").arg(num + 1)))
	{
		QFont fnt = pb->font();
		fnt.setBold(true);
		pb->setFont(fnt);
	}

	info("A FastAction %d is binded!", num + 1);
}

void PyOllyView::onClearBindingScriptsOfFastAction()
{
	QAction* const action = qobject_cast<QAction*>(sender());
	if (!action)
		return;
	const int num = action->property(kPropNumber.c_str()).toInt();

	do {
		auto ss = GlobalSettingsManger::instance().get();
		ss->settings->remove(kFastActionN.arg(num));
	} while (0);

	if (QPushButton* pb = findChild<QPushButton*>(QString("bFastAction%1").arg(num + 1)))
	{
		QFont fnt = pb->font();
		fnt.setBold(false);
		pb->setFont(fnt);
	}
}

