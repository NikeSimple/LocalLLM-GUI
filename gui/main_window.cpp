#include "main_window.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QFile>
#include <QProcess>
#include <QTimer>
#include "theme_manager.h"
#include "settings_dialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("LocalLLM-GUI");
    resize(1280, 800);

    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // ============================================================
    // Левая панель
    // ============================================================
    m_dialogList = new DialogListWidget(this);
    m_dialogList->setMinimumWidth(0);
    m_dialogList->setMaximumWidth(m_leftFullWidth);
    mainLayout->addWidget(m_dialogList, 0);

    // ============================================================
    // Центр — чат
    // ============================================================
    m_chat = new ChatWidget(this);
    mainLayout->addWidget(m_chat, 1);

    // ============================================================
    // Правая панель
    // ============================================================
    m_settings = new SettingsPanel(this);
    m_settings->setMinimumWidth(0);
    m_settings->setMaximumWidth(m_rightFullWidth);
    mainLayout->addWidget(m_settings, 0);

    setCentralWidget(central);

    // ============================================================
    // Анимации сворачивания
    // ============================================================
    m_leftAnim = new QPropertyAnimation(m_dialogList, "maximumWidth", this);
    m_leftAnim->setDuration(380);
    m_leftAnim->setEasingCurve(QEasingCurve::OutQuint);

    m_rightAnim = new QPropertyAnimation(m_settings, "maximumWidth", this);
    m_rightAnim->setDuration(380);
    m_rightAnim->setEasingCurve(QEasingCurve::OutQuint);

    // ============================================================
    // Связи ChatWidget
    // ============================================================
    connect(m_chat, &ChatWidget::messageSent,
            this, &MainWindow::onMessageSent);
    connect(m_chat, &ChatWidget::toggleLeftPanelRequested,
            this, &MainWindow::toggleLeftPanel);
    connect(m_chat, &ChatWidget::toggleRightPanelRequested,
            this, &MainWindow::toggleRightPanel);

    // ============================================================
    // Связи DialogListWidget
    // ============================================================
    connect(m_dialogList, &DialogListWidget::themeToggleRequested,
            this, [](){ ThemeManager::instance().toggle(); });
    connect(m_dialogList, &DialogListWidget::settingsRequested,
            this, &MainWindow::openSettingsDialog);

    // Переключение диалога
    connect(m_dialogList, &DialogListWidget::dialogSelected,
            this, [this](int row){
        m_chat->setCurrentDialog(row);
    });

    // Новый диалог — чистый чат
    connect(m_dialogList, &DialogListWidget::newDialogRequested,
            this, [this](){
        int newRow = m_dialogList->count() - 1;
        m_chat->setCurrentDialog(newRow);
    });

    // ============================================================
    // Перерисовка чата при смене темы
    // ============================================================
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](){
        m_chat->refreshTheme();
    });

    // ============================================================
    // API — Ollama
    // ============================================================
    m_api = new ApiClient(this);
    m_api->setBaseUrl("http://localhost:11434");

    connect(m_api, &ApiClient::pongReceived,
            this, &MainWindow::onPong);
    connect(m_api, &ApiClient::modelsReceived,
            this, &MainWindow::onModelsReceived);
    connect(m_api, &ApiClient::chatChunkReceived,
            this, &MainWindow::onChatChunk);
    connect(m_api, &ApiClient::chatFinished,
            this, &MainWindow::onChatFinished);
    connect(m_api, &ApiClient::chatError,
            this, [this](const QString &err){
        m_chat->appendSystemMessage(QString("Ошибка Ollama: %1").arg(err));
    });

    // Автогенерация названия диалога
    connect(m_api, &ApiClient::titleGenerated,
            this, [this](const QString &title){
        if (title.isEmpty()) return;
        int row = m_dialogList->currentRow();
        m_dialogList->updateDialogTitle(row, title);
        m_chat->setTitle(title);
    });

    // ============================================================
    // Стартовая проверка Ollama
    // ============================================================
    statusBar()->showMessage("Проверка подключения к Ollama...");
    m_chat->setStatusText("●  Проверка подключения...", "#a0a2b4");
    m_api->ping();
}

MainWindow::~MainWindow() = default;

// ============================================================
// Сворачивание панелей
// ============================================================

void MainWindow::toggleLeftPanel()
{
    m_leftCollapsed = !m_leftCollapsed;
    int from = m_dialogList->maximumWidth();
    int to = m_leftCollapsed ? 0 : m_leftFullWidth;

    m_leftAnim->stop();
    m_leftAnim->setStartValue(from);
    m_leftAnim->setEndValue(to);
    m_leftAnim->start();
}

void MainWindow::toggleRightPanel()
{
    m_rightCollapsed = !m_rightCollapsed;
    int from = m_settings->maximumWidth();
    int to = m_rightCollapsed ? 0 : m_rightFullWidth;

    m_rightAnim->stop();
    m_rightAnim->setStartValue(from);
    m_rightAnim->setEndValue(to);
    m_rightAnim->start();
}

void MainWindow::openSettingsDialog()
{
    SettingsDialog dlg(this);
    dlg.exec();
}

// ============================================================
// Ollama: ping и автозапуск
// ============================================================

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
    if (!localAppData.isEmpty())
        candidates << localAppData + "/Programs/Ollama/ollama.exe";

    QString programFiles = qgetenv("ProgramFiles");
    if (!programFiles.isEmpty())
        candidates << programFiles + "/Ollama/ollama.exe";

    QString foundPath;
    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            foundPath = path;
            break;
        }
    }

    if (foundPath.isEmpty()) {
        statusBar()->showMessage("Ollama не найдена · установите с ollama.com");
        m_chat->setStatusText("●  Ollama не найдена", "#ef6a7a");
        return;
    }

    QProcess *proc = new QProcess(this);
    proc->setProgram(foundPath);
    proc->setArguments({"serve"});
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->startDetached();

    QTimer::singleShot(4000, this, [this](){
        statusBar()->showMessage("Повторная проверка Ollama...");
        m_api->ping();
    });
}

void MainWindow::onModelsReceived(const QStringList &models)
{
    if (!models.isEmpty()) {
        m_settings->setAvailableModels(models);
    }
}

// ============================================================
// Отправка сообщения в Ollama
// ============================================================

void MainWindow::onMessageSent(const QString &text)
{
    // === Если ни один диалог не выбран — создаём новый ===
    if (m_dialogList->currentRow() < 0) {
        m_dialogList->createNewDialog();
    }

    QString filesContext = m_chat->attachedContext();

    if (text.isEmpty() && filesContext.isEmpty()) return;

    // Обновляем карточку диалога и просим автозаголовок
    int row = m_dialogList->currentRow();
    if (row >= 0) {
        if (!text.isEmpty()) {
            m_dialogList->updateDialogPreview(row, text);
        }

        static QSet<int> titledDialogs;
        if (!text.isEmpty() && !titledDialogs.contains(row)) {
            titledDialogs.insert(row);
            m_api->generateTitle(m_settings->currentModel(), text);
        }
    }

    // === Собираем запрос ===
    QJsonArray messages;

    QString sys = m_settings->systemPrompt().trimmed();

    if (!filesContext.isEmpty()) {
        sys += "\n\nНиже приложены файлы от пользователя. "
               "Используй их содержимое при ответе:";
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
        ? "Проанализируй приложенные файлы."
        : text;
    messages.append(userMsg);

    QJsonObject options;
    options["temperature"] = m_settings->temperature();
    options["top_p"] = m_settings->topP();
    options["num_ctx"] = m_settings->context();
    options["num_predict"] = m_settings->maxTokens();

    m_chat->beginAssistantMessage();
    m_api->sendChat(m_settings->currentModel(), messages, options);
}

// ============================================================
// Приём ответа
// ============================================================

void MainWindow::onChatChunk(const QString &chunk)
{
    m_chat->appendAssistantChunk(chunk);
}

void MainWindow::onChatFinished()
{
    m_chat->endAssistantMessage();
}