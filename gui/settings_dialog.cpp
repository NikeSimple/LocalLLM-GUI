#include "settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include "theme_manager.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Настройки");
    resize(640, 480);
    setObjectName("settingsDialog");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    QLabel *title = new QLabel("Настройки приложения", this);
    title->setObjectName("dialogTitle");
    layout->addWidget(title);

    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName("settingsTabs");

    m_tabs->addTab(buildAccountTab(), "Аккаунт");
    m_tabs->addTab(buildAppearanceTab(), "Оформление");
    m_tabs->addTab(buildServerTab(), "Сервер");
    m_tabs->addTab(buildDataTab(), "Данные");

    layout->addWidget(m_tabs, 1);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch();

    QPushButton *closeBtn = new QPushButton("Закрыть", this);
    closeBtn->setObjectName("primaryButton");
    closeBtn->setMinimumHeight(38);
    closeBtn->setMinimumWidth(120);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(closeBtn);

    layout->addLayout(buttons);
}

QWidget *SettingsDialog::buildAccountTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    QLabel *nameLabel = new QLabel("Имя профиля", tab);
    nameLabel->setObjectName("settingsLabel");
    layout->addWidget(nameLabel);

    QLineEdit *nameEdit = new QLineEdit("boba.mix999", tab);
    nameEdit->setObjectName("searchField");
    nameEdit->setMinimumHeight(38);
    layout->addWidget(nameEdit);

    QLabel *info = new QLabel(
        "Профиль хранится только на вашем устройстве.\n"
        "Аккаунт в облаке не создаётся.",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    layout->addWidget(info);

    layout->addStretch();
    return tab;
}

QWidget *SettingsDialog::buildAppearanceTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    QLabel *themeLabel = new QLabel("Тема оформления", tab);
    themeLabel->setObjectName("settingsLabel");
    layout->addWidget(themeLabel);

    QComboBox *themeCombo = new QComboBox(tab);
    themeCombo->setObjectName("modelSelector");
    themeCombo->setMinimumHeight(40);
    themeCombo->addItem("Светлая");
    themeCombo->addItem("Тёмная");
    if (ThemeManager::instance().isDark()) themeCombo->setCurrentIndex(1);
    layout->addWidget(themeCombo);

    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [](int idx){
        bool dark = (idx == 1);
        if (ThemeManager::instance().isDark() != dark) {
            ThemeManager::instance().toggle();
        }
    });

    QLabel *info = new QLabel("Смена темы применяется сразу же.", tab);
    info->setObjectName("paramHint");
    layout->addWidget(info);

    layout->addStretch();
    return tab;
}

QWidget *SettingsDialog::buildServerTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    QLabel *backendLabel = new QLabel("Программа для моделей", tab);
    backendLabel->setObjectName("settingsLabel");
    layout->addWidget(backendLabel);

    QComboBox *backendCombo = new QComboBox(tab);
    backendCombo->setObjectName("modelSelector");
    backendCombo->setMinimumHeight(40);
    backendCombo->addItem("Ollama (порт 11434)");
    backendCombo->addItem("llama.cpp (порт 8080)");
    layout->addWidget(backendCombo);

    QLabel *urlLabel = new QLabel("Адрес сервера", tab);
    urlLabel->setObjectName("settingsLabel");
    layout->addWidget(urlLabel);

    QLineEdit *urlEdit = new QLineEdit("http://localhost:11434", tab);
    urlEdit->setObjectName("searchField");
    urlEdit->setMinimumHeight(38);
    layout->addWidget(urlEdit);

    QPushButton *checkBtn = new QPushButton("Проверить и подключиться", tab);
    checkBtn->setObjectName("primaryButton");
    checkBtn->setMinimumHeight(40);
    layout->addWidget(checkBtn);

    layout->addStretch();
    return tab;
}

QWidget *SettingsDialog::buildDataTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    QLabel *exportLabel = new QLabel("Сохранить все диалоги", tab);
    exportLabel->setObjectName("settingsLabel");
    layout->addWidget(exportLabel);

    QPushButton *exportBtn = new QPushButton("Экспортировать в JSON", tab);
    exportBtn->setObjectName("secondaryButton");
    exportBtn->setMinimumHeight(40);
    layout->addWidget(exportBtn);

    QLabel *clearLabel = new QLabel("Очистить историю", tab);
    clearLabel->setObjectName("settingsLabel");
    layout->addWidget(clearLabel);

    QPushButton *clearBtn = new QPushButton("Удалить все диалоги", tab);
    clearBtn->setObjectName("secondaryButton");
    clearBtn->setMinimumHeight(40);
    layout->addWidget(clearBtn);

    QLabel *info = new QLabel(
        "Все данные хранятся только на вашем устройстве и никуда не отправляются.",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    layout->addWidget(info);

    layout->addStretch();
    return tab;
}