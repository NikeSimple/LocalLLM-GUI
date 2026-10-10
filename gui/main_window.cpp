#include "main_window.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QFile>
#include <QProcess>
#include <QTimer>
#include <QFileDialog>
#include <QJsonDocument>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include "theme_manager.h"
#include "../config/settings_manager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("LocalLLM-GUI");
    resize(1280, 800);

    QWidget *central = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(8, 8, 8, 8);
    centralLayout->setSpacing(8);

    QWidget *topArea = new QWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(topArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    m_dialogList = new DialogListWidget(this);
    m_dialogList->setMinimumWidth(0);
    m_dialogList->setMaximumWidth(m_leftFullWidth);
    mainLayout->addWidget(m_dialogList, 0);

    m_chat = new ChatWidget(this);
    mainLayout->addWidget(m_chat, 1);

    m_settings = new SettingsPanel(this);
    m_settings->setMinimumWidth(0);
    m_settings->setMaximumWidth(m_rightFullWidth);
    mainLayout->addWidget(m_settings, 0);

    centralLayout->addWidget(topArea, 1);

    m_settingsOverlay = new SettingsOverlay(central);
    m_settingsOverlay->setMaximumHeight(0);
    m_settingsOverlay->setMinimumHeight(0);
    centralLayout->addWidget(m_settingsOverlay, 0);

    setCentralWidget(central);

    m_leftAnim = new QPropertyAnimation(m_dialogList, "maximumWidth", this);
    m_leftAnim->setDuration(420);
    m_leftAnim->setEasingCurve(QEasingCurve::OutQuart);

    m_rightAnim = new QPropertyAnimation(m_settings, "maximumWidth", this);
    m_rightAnim->setDuration(420);
    m_rightAnim->setEasingCurve(QEasingCurve::OutQuart);

    m_settingsAnim = new QPropertyAnimation(m_settingsOverlay, "maximumHeight", this);
    m_settingsAnim->setDuration(380);
    m_settingsAnim->setEasingCurve(QEasingCurve::OutBack);

    // === БД в корне проекта ===
    m_db = new DatabaseManager(this);
    QString dbPath = QDir::current().absoluteFilePath("../localllm.db");
    if (m_db->open(dbPath)) {
        m_db->createTables();
        m_dialogRepo = new DialogRepository(m_db->database(), this);
        m_messageRepo = new MessageRepository(m_db->database(), this);
        m_promptRepo = new PromptRepository(m_db->database(), this);
        loadDialogsFromDb();
        statusBar()->showMessage(QString("БД: %1").arg(dbPath));
    } else {
        statusBar()->showMessage("Ошибка инициализации БД");
    }

    m_settingsOverlay->setRepositories(m_dialogRepo, m_messageRepo);
    m_settingsOverlay->setPromptRepository(m_promptRepo);

    // === Связи ChatWidget ===
    connect(m_chat, &ChatWidget::messageSent, this, &MainWindow::onMessageSent);
    connect(m_chat, &ChatWidget::messageAdded, this, &MainWindow::onMessageAdded);
    connect(m_chat, &ChatWidget::toggleLeftPanelRequested, this, &MainWindow::toggleLeftPanel);
    connect(m_chat, &ChatWidget::toggleRightPanelRequested, this, &MainWindow::toggleRightPanel);

    // === Связи DialogListWidget ===
    connect(m_dialogList, &DialogListWidget::themeToggleRequested,
            this, [](){ ThemeManager::instance().toggle(); });
    connect(m_dialogList, &DialogListWidget::settingsRequested,
            this, &MainWindow::toggleSettingsOverlay);
    connect(m_dialogList, &DialogListWidget::dialogSelected,
            this, &MainWindow::onDialogSelected);

    connect(m_dialogList, &DialogListWidget::newDialogRequested,
            this, [this](){
        int row = m_dialogList->count() - 1;
        if (row < 0) return;
        int dbId = m_dialogRepo->create("Новый диалог",
                                         m_settings->currentModel());
        m_dialogIndexToDbId[row] = dbId;
        m_chat->setCurrentDialog(row);
    });

    // === Удаление диалога из контекстного меню ===
    connect(m_dialogList, &DialogListWidget::deleteDialogRequested,
            this, [this](int row){
        if (!m_dialogRepo || !m_dialogIndexToDbId.contains(row)) return;

        auto reply = QMessageBox::question(
            this, "Удаление диалога",
            "Удалить этот диалог? Сообщения будут удалены безвозвратно.",
            QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes) return;

        int dbId = m_dialogIndexToDbId[row];

        m_dialogRepo->remove(dbId);
        m_dialogIndexToDbId.remove(row);

        m_dialogList->removeDialogRow(row);

        QMap<int, int> newMap;
        for (auto it = m_dialogIndexToDbId.begin();
             it != m_dialogIndexToDbId.end(); ++it) {
            if (it.key() > row) {
                newMap[it.key() - 1] = it.value();
            } else {
                newMap[it.key()] = it.value();
            }
        }
        m_dialogIndexToDbId = newMap;

        if (m_dialogList->currentRow() < 0) {
            m_chat->clear();
        }

        statusBar()->showMessage("Диалог удалён");
    });

    // === Экспорт диалога из контекстного меню ===
    connect(m_dialogList, &DialogListWidget::exportDialogRequested,
            this, &MainWindow::exportDialogByRow);

    // === Экспорт из тулбара чата ===
    connect(m_chat, &ChatWidget::exportDialogRequested, this, [this](int dialogIndex){
        Q_UNUSED(dialogIndex);
        int row = m_dialogList->currentRow();
        exportDialogByRow(row);
    });

    // === Связи SettingsOverlay ===
    connect(m_settingsOverlay, &SettingsOverlay::closeRequested,
            this, &MainWindow::toggleSettingsOverlay);
    connect(m_settingsOverlay, &SettingsOverlay::profileNameChanged,
            m_dialogList, &DialogListWidget::setUserName);
    connect(m_settingsOverlay, &SettingsOverlay::clearHistoryRequested,
            this, [this](){
        m_dialogList->clearAllDialogs();
        m_chat->clear();
        m_dialogIndexToDbId.clear();
    });

    connect(m_settingsOverlay, &SettingsOverlay::promptApplied,
            this, [this](const QString &text){
        m_settings->applySystemPrompt(text);
    });

    // === Смена темы ===
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](){ m_chat->refreshTheme(); });

    // === API ===
    m_api = new ApiClient(this);
    m_api->setBaseUrl("http://localhost:11434");

    connect(m_api, &ApiClient::pongReceived, this, &MainWindow::onPong);
    connect(m_api, &ApiClient::modelsReceived, this, &MainWindow::onModelsReceived);
    connect(m_api, &ApiClient::chatChunkReceived, this, &MainWindow::onChatChunk);
    connect(m_api, &ApiClient::chatFinished, this, &MainWindow::onChatFinished);
    connect(m_api, &ApiClient::chatError, this, [this](const QString &err){
        m_chat->appendSystemMessage(QString("Ошибка Ollama: %1").arg(err));
    });
    connect(m_api, &ApiClient::titleGenerated, this, &MainWindow::onTitleGenerated);

    statusBar()->showMessage("Проверка подключения к Ollama...");
    m_chat->setStatusText("●  Проверка подключения...", "#a0a2b4");
    m_api->ping();
}

MainWindow::~MainWindow() = default;

void MainWindow::exportDialogByRow(int row)
{
    if (!m_dialogRepo || !m_messageRepo) return;
    if (!m_dialogIndexToDbId.contains(row)) return;

    QString path = QFileDialog::getSaveFileName(
        this, "Экспортировать диалог",
        QString("dialog-%1.json")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmm")),
        "JSON файлы (*.json)");
    if (path.isEmpty()) return;

    int dbId = m_dialogIndexToDbId[row];
    DialogInfo info = m_dialogRepo->getById(dbId);

    QJsonObject root;
    root["title"] = info.title;
    root["created_at"] = info.createdAt;
    root["updated_at"] = info.updatedAt;
    root["model_name"] = info.modelName;

    QJsonArray messagesArray;
    for (const MessageInfo &m : m_messageRepo->listByDialog(dbId)) {
        QJsonObject msgObj;
        msgObj["role"] = m.role;
        msgObj["content"] = m.content;
        msgObj["created_at"] = m.createdAt;
        messagesArray.append(msgObj);
    }
    root["messages"] = messagesArray;

    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка",
            QString("Не удалось создать файл: %1").arg(path));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Готово",
        QString("Диалог экспортирован в %1").arg(path));
}

void MainWindow::toggleSettingsOverlay()
{
    m_settingsOpen = !m_settingsOpen;
    m_settingsAnim->stop();
    m_settingsAnim->setStartValue(m_settingsOverlay->maximumHeight());
    m_settingsAnim->setEndValue(m_settingsOpen ? m_settingsOverlayHeight : 0);
    m_settingsAnim->start();
}

void MainWindow::loadDialogsFromDb()
{
    if (!m_dialogRepo) return;
    QVector<DialogInfo> dialogs = m_dialogRepo->listAll();
    for (const DialogInfo &info : dialogs) {
        int row = m_dialogList->count();
        m_dialogList->addDialogCardStatic(info.title, "Загружено из БД", "");
        m_dialogIndexToDbId[row] = info.id;
    }
    qDebug() << "Загружено диалогов из БД:" << dialogs.size();
}

void MainWindow::onDialogSelected(int row)
{
    if (!m_dialogIndexToDbId.contains(row)) {
        m_chat->setCurrentDialog(row);
        return;
    }
    int dbId = m_dialogIndexToDbId[row];
    QVector<MessageInfo> msgs = m_messageRepo->listByDialog(dbId);
    QVector<ChatMessage> chatMsgs;
    for (const MessageInfo &m : msgs) {
        ChatMessage cm;
        cm.role = m.role;
        cm.text = m.content;
        chatMsgs.append(cm);
    }
    m_chat->loadMessages(row, chatMsgs);
}

void MainWindow::onMessageAdded(int dialogIndex, const QString &role, const QString &text)
{
    if (!m_messageRepo || !m_dialogIndexToDbId.contains(dialogIndex)) return;
    int dbId = m_dialogIndexToDbId[dialogIndex];
    m_messageRepo->add(dbId, role, text);
    m_dialogRepo->touch(dbId);
}

void MainWindow::onTitleGenerated(const QString &title)
{
    if (title.isEmpty()) return;
    int row = m_dialogList->currentRow();
    m_dialogList->updateDialogTitle(row, title);
    m_chat->setTitle(title);
    if (m_dialogRepo && m_dialogIndexToDbId.contains(row)) {
        m_dialogRepo->updateTitle(m_dialogIndexToDbId[row], title);
    }
}

void MainWindow::onPong(bool ok, int count)
{
    if (ok) {
        statusBar()->showMessage(QString("Подключено: %1 моделей").arg(count));
        m_chat->setStatusText(QString("●  Подключено · %1 моделей").arg(count), "#3fbf8a");
        m_api->listModels();
        return;
    }

    statusBar()->showMessage("Запуск Ollama...");
    m_chat->setStatusText("●  Запуск Ollama...", "#a0a2b4");

    QStringList candidates;
    QString localAppData = qgetenv("LOCALAPPDATA");
    if (!localAppData.isEmpty()) candidates << localAppData + "/Programs/Ollama/ollama.exe";
    QString programFiles = qgetenv("ProgramFiles");
    if (!programFiles.isEmpty()) candidates << programFiles + "/Ollama/ollama.exe";

    QString foundPath;
    for (const QString &path : candidates) {
        if (QFile::exists(path)) { foundPath = path; break; }
    }
    if (foundPath.isEmpty()) {
        statusBar()->showMessage("Ollama не найдена");
        return;
    }
    QProcess *proc = new QProcess(this);
    proc->setProgram(foundPath);
    proc->setArguments({"serve"});
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->startDetached();
    QTimer::singleShot(4000, this, [this](){ m_api->ping(); });
}

void MainWindow::onModelsReceived(const QStringList &models)
{
    if (!models.isEmpty()) m_settings->setAvailableModels(models);
}

void MainWindow::onMessageSent(const QString &text)
{
    if (m_dialogList->currentRow() < 0) {
        m_dialogList->createNewDialog();
    }

    QString filesContext = m_chat->attachedContext();
    if (text.isEmpty() && filesContext.isEmpty()) return;

    int row = m_dialogList->currentRow();
    if (row >= 0 && !text.isEmpty()) {
        m_dialogList->updateDialogPreview(row, text);
        static QSet<int> titledDialogs;
        if (!titledDialogs.contains(row)) {
            titledDialogs.insert(row);
            m_api->generateTitle(m_settings->currentModel(), text);
        }
    }

    QJsonArray messages;
    QString sys = m_settings->systemPrompt().trimmed();
    if (!filesContext.isEmpty()) {
        sys += "\n\nНиже приложены файлы от пользователя. "
               "Обязательно используй их содержимое при ответе:\n";
        sys += filesContext;
    }
    if (!sys.isEmpty()) {
        QJsonObject sysMsg;
        sysMsg["role"] = "system";
        sysMsg["content"] = sys;
        messages.append(sysMsg);
    }
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = text.isEmpty()
        ? "Проанализируй приложенные файлы и ответь по их содержимому."
        : text;
    messages.append(userMsg);

    QJsonObject options;
    options["temperature"] = m_settings->temperature();
    options["top_p"] = m_settings->topP();
    options["num_ctx"] = m_settings->context();
    options["num_predict"] = m_settings->maxTokens();
    options["repeat_penalty"] = 1.1;
    options["frequency_penalty"] = 0.3;
    options["presence_penalty"] = 0.3;

    m_chat->beginAssistantMessage();
    m_api->sendChat(m_settings->currentModel(), messages, options);

    m_chat->clearAttachments();
}

void MainWindow::onChatChunk(const QString &chunk)
{
    m_chat->appendAssistantChunk(chunk);
}

void MainWindow::onChatFinished()
{
    m_chat->endAssistantMessage();
}

void MainWindow::toggleLeftPanel()
{
    m_leftCollapsed = !m_leftCollapsed;
    m_leftAnim->stop();
    m_leftAnim->setStartValue(m_dialogList->maximumWidth());
    m_leftAnim->setEndValue(m_leftCollapsed ? 0 : m_leftFullWidth);
    m_leftAnim->start();
}

void MainWindow::toggleRightPanel()
{
    m_rightCollapsed = !m_rightCollapsed;
    m_rightAnim->stop();
    m_rightAnim->setStartValue(m_settings->maximumWidth());
    m_rightAnim->setEndValue(m_rightCollapsed ? 0 : m_rightFullWidth);
    m_rightAnim->start();
}