#include "config.h"
#include "ui_configui.h"

config::config(const QString& configLocation, QWidget* parent /* = nullptr */) : QDialog(parent),
	m_ui(std::make_unique<Ui::configui>()),
	m_settings(std::make_unique<QSettings>(configLocation + "/TelegramWatchdog.ini", QSettings::IniFormat, this))
{
	m_ui->setupUi(this);

	setWindowTitle("Telegram Watchdog :: Config");

	// Connect UI Elements.
	connect(m_ui->pbOk, &QPushButton::clicked, this, &config::saveSettings);
	connect(m_ui->pbCancel, &QPushButton::clicked, this, &QWidget::close);
	// Show off some Lambda connect

	connect(m_ui->integratedBot, &QCheckBox::toggled, this, [&](bool checked) {
		m_ui->BotTokenInput->setEnabled(!checked);
		m_ui->floodProtection->setEnabled(!checked);
		});
	adjustSize();
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

config::~config() {
	m_settings->sync();
}


void config::setConfigOption(const QString& option, const QVariant& value) {
	m_settings->setValue(option, value);
}

QVariant config::getConfigOption(const QString& option) const {
	return m_settings->value(option);
}

void config::showEvent(QShowEvent* /* e */) {
	adjustSize();
	loadSettings();
}

void config::changeEvent(QEvent* e) {
	if (e->type() == QEvent::StyleChange && isVisible()) {
		m_ui->verticalLayoutWidget_2->adjustSize();
		adjustSize();
	}
}

void config::saveSettings() {
	setConfigOption("botToken", m_ui->BotTokenInput->text());
	setConfigOption("chatID", m_ui->ChatIDInput->text());
	setConfigOption("greetings", m_ui->greetings->isChecked());
	setConfigOption("integratedBot", m_ui->integratedBot->isChecked());
	setConfigOption("floodProtection", m_ui->floodProtection->isChecked());
	setConfigOption("channels2Watch", m_ui->channels2Watch->text());
	setConfigOption("servers2Watch", m_ui->servers2Watch->text());
	setConfigOption("forced", m_ui->forced->isChecked());

	close();
}

void config::loadSettings() {
	m_ui->BotTokenInput->setText(getConfigOption("botToken").toString());
	m_ui->ChatIDInput->setText(getConfigOption("chatID").toString());
	m_ui->greetings->setChecked(getConfigOption("greetings").toBool());
	m_ui->integratedBot->setChecked(getConfigOption("integratedBot").toBool());
	m_ui->floodProtection->setChecked(getConfigOption("floodProtection").toBool());
	m_ui->channels2Watch->setText(getConfigOption("channels2Watch").toString());
	m_ui->servers2Watch->setText(getConfigOption("servers2Watch").toString());
	m_ui->forced->setChecked(getConfigOption("forced").toBool());

	m_ui->BotTokenInput->setEnabled(!m_ui->integratedBot->isChecked());
	m_ui->floodProtection->setEnabled(!m_ui->integratedBot->isChecked());
}