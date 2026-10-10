#include "settings_overlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QFrame>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include "theme_manager.h"
#include "../config/settings_manager.h"

SettingsOverlay::SettingsOverlay(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("settingsOverlay");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 14, 20, 14);
    layout->setSpacing(10);

    QLabel *title = new QLabel("Настройки приложения", this);
    title->setObjectName("dialogTitle");
    layout->addWidget(title);

    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName("settingsTabs");

    m_tabs->addTab(buildAccountTab(), "Аккаунт");
    m_tabs->addTab(buildAppearanceTab(), "Оформление");
    m_tabs->addTab(buildServerTab(), "Сервер");
    m_tabs->addTab(buildTemplatesTab(), "Шаблоны");
    m_tabs->addTab(buildDataTab(), "Данные");

    layout->addWidget(m_tabs, 1);
}

void SettingsOverlay::setRepositories(DialogRepository *dialogRepo,
                                       MessageRepository *messageRepo)
{
    m_dialogRepo = dialogRepo;
    m_messageRepo = messageRepo;
}

void SettingsOverlay::setPromptRepository(PromptRepository *promptRepo)
{
    m_promptRepo = promptRepo;
    refreshTemplatesList();
    collectAndEmitActivePrompts();
}

QWidget *SettingsOverlay::buildAccountTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    QVBoxLayout *left = new QVBoxLayout();
    QLabel *nameLabel = new QLabel("Имя профиля", tab);
    nameLabel->setObjectName("settingsLabel");
    left->addWidget(nameLabel);

    m_nameEdit = new QLineEdit(tab);
    m_nameEdit->setObjectName("searchField");
    m_nameEdit->setMinimumHeight(38);
    m_nameEdit->setPlaceholderText("Введите имя");
    left->addWidget(m_nameEdit);

    QPushButton *saveBtn = new QPushButton("Сохранить имя", tab);
    saveBtn->setObjectName("primaryButton");
    saveBtn->setMinimumHeight(38);
    connect(saveBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onSaveNameClicked);
    left->addWidget(saveBtn);

    m_nameStatus = new QLabel("", tab);
    m_nameStatus->setObjectName("paramHint");
    m_nameStatus->setWordWrap(true);
    left->addWidget(m_nameStatus);
    left->addStretch();

    layout->addLayout(left, 1);

    QLabel *info = new QLabel(
        "Профиль хранится только на вашем устройстве.\n"
        "Аккаунт в облаке не создаётся.",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    layout->addWidget(info, 1);

    return tab;
}

QWidget *SettingsOverlay::buildAppearanceTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    QVBoxLayout *left = new QVBoxLayout();
    QLabel *themeLabel = new QLabel("Тема оформления", tab);
    themeLabel->setObjectName("settingsLabel");
    left->addWidget(themeLabel);

    QComboBox *themeCombo = new QComboBox(tab);
    themeCombo->setObjectName("modelSelector");
    themeCombo->setMinimumHeight(40);

    for (int i = 0; i <= 3; ++i) {
        ThemeManager::Theme t = static_cast<ThemeManager::Theme>(i);
        themeCombo->addItem(ThemeManager::themeDisplayName(t));
    }
    themeCombo->setCurrentIndex(static_cast<int>(ThemeManager::instance().currentTheme()));

    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [](int idx){
        ThemeManager::Theme t = static_cast<ThemeManager::Theme>(idx);
        ThemeManager::instance().setTheme(t);
        SettingsManager::instance().setValue("theme", QString::number(idx));
    });

    left->addWidget(themeCombo);
    left->addStretch();
    layout->addLayout(left, 1);

    QLabel *info = new QLabel(
        "Доступны 4 темы:\n"
        "• Светлая\n"
        "• Тёмная\n"
        "• Красная\n"
        "• Океан",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    layout->addWidget(info, 1);

    return tab;
}

QWidget *SettingsOverlay::buildServerTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    QVBoxLayout *left = new QVBoxLayout();
    QLabel *backendLabel = new QLabel("Программа для моделей", tab);
    backendLabel->setObjectName("settingsLabel");
    left->addWidget(backendLabel);

    QComboBox *backendCombo = new QComboBox(tab);
    backendCombo->setObjectName("modelSelector");
    backendCombo->setMinimumHeight(40);
    backendCombo->addItem("Ollama (порт 11434)");
    backendCombo->addItem("llama.cpp (порт 8080)");
    left->addWidget(backendCombo);

    QLabel *urlLabel = new QLabel("Адрес сервера", tab);
    urlLabel->setObjectName("settingsLabel");
    left->addWidget(urlLabel);

    m_urlEdit = new QLineEdit("http://localhost:11434", tab);
    m_urlEdit->setObjectName("searchField");
    m_urlEdit->setMinimumHeight(38);
    left->addWidget(m_urlEdit);

    QPushButton *checkBtn = new QPushButton("Проверить и подключиться", tab);
    checkBtn->setObjectName("primaryButton");
    checkBtn->setMinimumHeight(40);
    connect(checkBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onCheckConnectionClicked);
    left->addWidget(checkBtn);

    m_serverStatus = new QLabel("", tab);
    m_serverStatus->setObjectName("paramHint");
    m_serverStatus->setWordWrap(true);
    left->addWidget(m_serverStatus);
    left->addStretch();

    layout->addLayout(left, 1);

    QLabel *info = new QLabel(
        "Проверка подключения — GET-запрос к /api/tags.\n"
        "Если сервер отвечает, он доступен.",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    layout->addWidget(info, 1);

    return tab;
}

QWidget *SettingsOverlay::buildTemplatesTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // Левая часть — список шаблонов с чекбоксами
    QVBoxLayout *left = new QVBoxLayout();
    QLabel *listLabel = new QLabel("Сохранённые шаблоны", tab);
    listLabel->setObjectName("settingsLabel");
    left->addWidget(listLabel);

    QLabel *checkHint = new QLabel(
        "Отметьте галочками шаблоны, которые должны быть активны. "
        "Их тексты объединятся и попадут в поле «Инструкция для модели».",
        tab);
    checkHint->setObjectName("paramHint");
    checkHint->setWordWrap(true);
    left->addWidget(checkHint);

    m_templatesList = new QListWidget(tab);
    m_templatesList->setObjectName("dialogsList");
    m_templatesList->setMinimumHeight(180);
    left->addWidget(m_templatesList);

    QHBoxLayout *btns = new QHBoxLayout();
    QPushButton *createBtn = new QPushButton("Создать", tab);
    createBtn->setObjectName("primaryButton");
    createBtn->setMinimumHeight(36);
    connect(createBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onCreateTemplateClicked);
    btns->addWidget(createBtn);

    QPushButton *editBtn = new QPushButton("Изменить", tab);
    editBtn->setObjectName("secondaryButton");
    editBtn->setMinimumHeight(36);
    connect(editBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onEditTemplateClicked);
    btns->addWidget(editBtn);

    QPushButton *deleteBtn = new QPushButton("Удалить", tab);
    deleteBtn->setObjectName("secondaryButton");
    deleteBtn->setMinimumHeight(36);
    connect(deleteBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onDeleteTemplateClicked);
    btns->addWidget(deleteBtn);

    left->addLayout(btns);
    layout->addLayout(left, 2);

    // Правая часть — применение
    QVBoxLayout *right = new QVBoxLayout();
    QLabel *applyLabel = new QLabel("Активные шаблоны", tab);
    applyLabel->setObjectName("settingsLabel");
    right->addWidget(applyLabel);

    QLabel *info = new QLabel(
        "Активные шаблоны автоматически объединяются и подставляются "
        "в поле «Инструкция для модели» в правой панели. "
        "Порядок — сверху вниз по списку.\n\n"
        "Чтобы отключить все шаблоны — снимите все галочки.",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    right->addWidget(info);

    QPushButton *applyBtn = new QPushButton("Обновить активные", tab);
    applyBtn->setObjectName("primaryButton");
    applyBtn->setMinimumHeight(40);
    connect(applyBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onApplyTemplateClicked);
    right->addWidget(applyBtn);
    right->addStretch();

    layout->addLayout(right, 1);

    return tab;
}

QWidget *SettingsOverlay::buildDataTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    QVBoxLayout *left = new QVBoxLayout();

    int dialogCount = m_dialogRepo ? m_dialogRepo->listAll().size() : 0;
    m_statsLabel = new QLabel(
        QString("Всего диалогов: %1").arg(dialogCount), tab);
    m_statsLabel->setObjectName("paramLabel");
    left->addWidget(m_statsLabel);

    QPushButton *exportBtn = new QPushButton("Экспортировать в JSON", tab);
    exportBtn->setObjectName("secondaryButton");
    exportBtn->setMinimumHeight(40);
    connect(exportBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onExportClicked);
    left->addWidget(exportBtn);

    QPushButton *clearBtn = new QPushButton("Удалить все диалоги", tab);
    clearBtn->setObjectName("secondaryButton");
    clearBtn->setMinimumHeight(40);
    connect(clearBtn, &QPushButton::clicked,
            this, &SettingsOverlay::onClearHistoryClicked);
    left->addWidget(clearBtn);
    left->addStretch();

    layout->addLayout(left, 1);

    QLabel *info = new QLabel(
        "Все данные хранятся только на вашем устройстве\n"
        "и никуда не отправляются.",
        tab);
    info->setObjectName("paramHint");
    info->setWordWrap(true);
    layout->addWidget(info, 1);

    return tab;
}

// ============================================================
// Обработчики
// ============================================================

void SettingsOverlay::onSaveNameClicked()
{
    QString newName = m_nameEdit->text().trimmed();
    if (newName.length() < 2) {
        m_nameStatus->setText("Имя должно содержать минимум 2 символа");
        m_nameStatus->setStyleSheet("color: #ef6a7a;");
        return;
    }
    emit profileNameChanged(newName);
    SettingsManager::instance().setValue("userName", newName);
    m_nameStatus->setText("Имя сохранено: " + newName);
    m_nameStatus->setStyleSheet("color: #3fbf8a;");
}

void SettingsOverlay::onCheckConnectionClicked()
{
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) url = "http://localhost:11434";
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
    }

    m_serverStatus->setText("Проверка...");
    m_serverStatus->setStyleSheet("color: #a0a2b4;");

    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl(url + "/api/tags"));

    QNetworkReply *reply = mgr->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, mgr](){
        reply->deleteLater();
        mgr->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            m_serverStatus->setText(
                QString("❌ Ошибка: %1").arg(reply->errorString()));
            m_serverStatus->setStyleSheet("color: #ef6a7a;");
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray arr = doc.object().value("models").toArray();

        m_serverStatus->setText(
            QString("✅ Подключено · моделей: %1").arg(arr.size()));
        m_serverStatus->setStyleSheet("color: #3fbf8a;");
        SettingsManager::instance().setValue("serverUrl", m_urlEdit->text());
    });
}

void SettingsOverlay::onExportClicked()
{
    if (!m_dialogRepo || !m_messageRepo) {
        QMessageBox::warning(this, "Ошибка", "Репозитории не подключены");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить диалоги",
        QString("localllm-dialogs-%1.json")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")),
        "JSON файлы (*.json)");
    if (path.isEmpty()) return;

    QJsonArray dialogsArray;
    QVector<DialogInfo> dialogs = m_dialogRepo->listAll();

    for (const DialogInfo &d : dialogs) {
        QJsonObject dialogObj;
        dialogObj["id"] = d.id;
        dialogObj["title"] = d.title;
        dialogObj["created_at"] = d.createdAt;
        dialogObj["updated_at"] = d.updatedAt;
        dialogObj["model_name"] = d.modelName;

        QJsonArray messagesArray;
        for (const MessageInfo &m : m_messageRepo->listByDialog(d.id)) {
            QJsonObject msgObj;
            msgObj["role"] = m.role;
            msgObj["content"] = m.content;
            msgObj["created_at"] = m.createdAt;
            messagesArray.append(msgObj);
        }
        dialogObj["messages"] = messagesArray;
        dialogsArray.append(dialogObj);
    }

    QJsonObject root;
    root["exported_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["dialogs"] = dialogsArray;

    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка",
            QString("Не удалось создать файл: %1").arg(path));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Готово",
        QString("Экспортировано %1 диалогов").arg(dialogsArray.size()));
}

void SettingsOverlay::onClearHistoryClicked()
{
    if (!m_dialogRepo) return;

    auto reply = QMessageBox::question(
        this, "Удаление всех диалогов",
        "Вы уверены? Это действие нельзя отменить.",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    int count = 0;
    for (const DialogInfo &d : m_dialogRepo->listAll()) {
        if (m_dialogRepo->remove(d.id)) count++;
    }

    QMessageBox::information(this, "Готово",
        QString("Удалено диалогов: %1").arg(count));

    if (m_statsLabel) m_statsLabel->setText("Всего диалогов: 0");
    emit clearHistoryRequested();
}

// ============================================================
// Шаблоны промптов
// ============================================================

void SettingsOverlay::refreshTemplatesList()
{
    if (!m_templatesList) return;
    m_templatesList->clear();
    if (!m_promptRepo) return;

    for (const PromptInfo &p : m_promptRepo->listAll()) {
        QListWidgetItem *item = new QListWidgetItem(p.name);
        item->setData(Qt::UserRole, p.id);
        item->setToolTip(p.text.left(200));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(p.isTemplate ? Qt::Checked : Qt::Unchecked);
        m_templatesList->addItem(item);
    }

    // Обработчик изменения чекбоксов — подключаем один раз
    static bool connected = false;
    if (!connected) {
        connect(m_templatesList, &QListWidget::itemChanged,
                this, [this](QListWidgetItem *item){
            if (!item) return;
            int id = item->data(Qt::UserRole).toInt();
            bool checked = (item->checkState() == Qt::Checked);
            onTemplateCheckChanged(id, checked);
        });
        connected = true;
    }
}

void SettingsOverlay::onTemplateCheckChanged(int id, bool checked)
{
    if (!m_promptRepo) return;
    m_promptRepo->setActive(id, checked);
    collectAndEmitActivePrompts();
}

void SettingsOverlay::collectAndEmitActivePrompts()
{
    if (!m_promptRepo) return;

    QStringList activeTexts;
    QVector<PromptInfo> all = m_promptRepo->listAll();

    // listAll возвращает ORDER BY id DESC, значит сверху — самые новые.
    // Чтобы порядок был «сверху вниз как в списке», обходим в том же порядке.
    for (const PromptInfo &p : all) {
        if (p.isTemplate) {
            activeTexts.append(p.text.trimmed());
        }
    }

    QString combined = activeTexts.join("\n\n---\n\n");
    emit promptApplied(combined);
}

void SettingsOverlay::onCreateTemplateClicked()
{
    if (!m_promptRepo) {
        QMessageBox::warning(this, "Ошибка", "Репозиторий не подключён");
        return;
    }

    bool ok = false;
    QString name = QInputDialog::getText(this, "Новый шаблон",
        "Название шаблона:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    QString text = QInputDialog::getMultiLineText(this, "Текст шаблона",
        "Введите промпт (можно использовать перенос строки Enter):",
        "", &ok);
    if (!ok || text.trimmed().isEmpty()) return;

    m_promptRepo->create(name.trimmed(), text.trimmed());
    refreshTemplatesList();
    collectAndEmitActivePrompts();
}

void SettingsOverlay::onEditTemplateClicked()
{
    if (!m_promptRepo || !m_templatesList) return;
    QListWidgetItem *item = m_templatesList->currentItem();
    if (!item) {
        QMessageBox::information(this, "Выбор", "Выберите шаблон для редактирования");
        return;
    }

    int id = item->data(Qt::UserRole).toInt();
    PromptInfo p = m_promptRepo->getById(id);
    if (p.id < 0) return;

    bool ok = false;
    QString name = QInputDialog::getText(this, "Редактирование",
        "Название:", QLineEdit::Normal, p.name, &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    QString text = QInputDialog::getMultiLineText(this, "Текст шаблона",
        "Промпт (можно использовать перенос строки Enter):", p.text, &ok);
    if (!ok) return;

    m_promptRepo->update(id, name.trimmed(), text.trimmed());
    refreshTemplatesList();
    collectAndEmitActivePrompts();
}

void SettingsOverlay::onDeleteTemplateClicked()
{
    if (!m_promptRepo || !m_templatesList) return;
    QListWidgetItem *item = m_templatesList->currentItem();
    if (!item) {
        QMessageBox::information(this, "Выбор", "Выберите шаблон для удаления");
        return;
    }

    int id = item->data(Qt::UserRole).toInt();

    auto reply = QMessageBox::question(this, "Удаление шаблона",
        "Удалить этот шаблон?", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    m_promptRepo->remove(id);
    refreshTemplatesList();
    collectAndEmitActivePrompts();
}

void SettingsOverlay::onApplyTemplateClicked()
{
    collectAndEmitActivePrompts();
    QMessageBox::information(this, "Готово",
        "Активные шаблоны применены в поле «Инструкция для модели»");
}