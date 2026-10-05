#include "chat_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QTextCursor>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include "theme_manager.h"
#include "../core/file_handler.h"

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    // ============================================================
    // Тулбар
    // ============================================================
    QWidget *toolbar = new QWidget(this);
    toolbar->setObjectName("chatToolbar");
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(6, 6, 6, 6);
    toolbarLayout->setSpacing(8);

    m_toggleLeftBtn = new QPushButton("☰", toolbar);
    m_toggleLeftBtn->setObjectName("iconButton");
    m_toggleLeftBtn->setFixedSize(32, 32);
    m_toggleLeftBtn->setCursor(Qt::PointingHandCursor);
    m_toggleLeftBtn->setToolTip("Свернуть/развернуть левую панель");
    connect(m_toggleLeftBtn, &QPushButton::clicked,
            this, &ChatWidget::toggleLeftPanelRequested);

    m_titleLabel = new QLabel("Новый диалог", toolbar);
    m_titleLabel->setObjectName("chatTitle");

    m_statusLabel = new QLabel("●  Не подключено", toolbar);
    m_statusLabel->setObjectName("chatStatus");

    QPushButton *copyBtn = new QPushButton("⧉", toolbar);
    copyBtn->setObjectName("iconButton");
    copyBtn->setFixedSize(32, 32);
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setToolTip("Скопировать весь чат в буфер");
    connect(copyBtn, &QPushButton::clicked,
            this, &ChatWidget::onCopyAllClicked);

    m_toggleRightBtn = new QPushButton("☰", toolbar);
    m_toggleRightBtn->setObjectName("iconButton");
    m_toggleRightBtn->setFixedSize(32, 32);
    m_toggleRightBtn->setCursor(Qt::PointingHandCursor);
    m_toggleRightBtn->setToolTip("Свернуть/развернуть правую панель");
    connect(m_toggleRightBtn, &QPushButton::clicked,
            this, &ChatWidget::toggleRightPanelRequested);

    toolbarLayout->addWidget(m_toggleLeftBtn);
    toolbarLayout->addSpacing(4);
    toolbarLayout->addWidget(m_titleLabel);
    toolbarLayout->addSpacing(10);
    toolbarLayout->addWidget(m_statusLabel);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(copyBtn);
    toolbarLayout->addSpacing(4);
    toolbarLayout->addWidget(m_toggleRightBtn);

    layout->addWidget(toolbar);

    QFrame *line = new QFrame(this);
    line->setObjectName("toolbarSeparator");
    line->setFrameShape(QFrame::HLine);
    layout->addWidget(line);

    // ============================================================
    // Чат
    // ============================================================
    m_chatView = new QTextEdit(this);
    m_chatView->setReadOnly(true);
    m_chatView->setPlaceholderText("История диалога появится здесь...");
    m_chatView->setObjectName("chatView");
    layout->addWidget(m_chatView, 1);

    // ============================================================
    // Чипы файлов
    // ============================================================
    m_chipsRow = new QWidget(this);
    m_chipsRow->setObjectName("chipsRow");
    m_chipsLayout = new QHBoxLayout(m_chipsRow);
    m_chipsLayout->setContentsMargins(0, 0, 0, 0);
    m_chipsLayout->setSpacing(6);
    m_chipsRow->setVisible(false);
    layout->addWidget(m_chipsRow);

    // ============================================================
    // Поле ввода
    // ============================================================
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(8);

    m_attachButton = new QPushButton("📎", this);
    m_attachButton->setObjectName("attachButton");
    m_attachButton->setFixedSize(38, 38);
    m_attachButton->setCursor(Qt::PointingHandCursor);
    m_attachButton->setToolTip("Прикрепить файл");
    connect(m_attachButton, &QPushButton::clicked,
            this, &ChatWidget::onAttachClicked);
    inputLayout->addWidget(m_attachButton);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Введите сообщение... (Enter — отправить)");
    connect(m_input, &QLineEdit::returnPressed,
            this, &ChatWidget::onSendClicked);
    inputLayout->addWidget(m_input);

    m_sendButton = new QPushButton("Отправить", this);
    m_sendButton->setObjectName("sendButton");
    connect(m_sendButton, &QPushButton::clicked,
            this, &ChatWidget::onSendClicked);
    inputLayout->addWidget(m_sendButton);

    layout->addLayout(inputLayout);
}

// ============================================================
// Цвета по теме
// ============================================================
static QString bodyColor()
{
    return ThemeManager::instance().isDark() ? "#f2f2f7" : "#1c1d2b";
}

static QString userLabelColor() { return "#3f6fd4"; }
static QString assistantLabelColor() { return "#6a97f2"; }
static QString systemLabelColor() { return "#a0a2b4"; }

void ChatWidget::refreshTheme()
{
    renderCurrentDialog();
}

// ============================================================
// Работа с диалогами
// ============================================================

void ChatWidget::setCurrentDialog(int index)
{
    if (!m_dialogs.contains(index)) {
        m_dialogs[index] = QVector<ChatMessage>();
    }

    m_currentDialog = index;
    renderCurrentDialog();
    setTitle(QString("Диалог %1").arg(index + 1));
}

void ChatWidget::renderCurrentDialog()
{
    if (m_currentDialog < 0) {
        m_chatView->clear();
        return;
    }

    const QVector<ChatMessage> &messages = m_dialogs.value(m_currentDialog);

    if (messages.isEmpty() && !m_streaming) {
        m_chatView->clear();
        return;
    }

    QString color = bodyColor();
    QString codeBg = ThemeManager::instance().isDark() ? "#1b1c23" : "#f4f6fb";
    QString codeColor = ThemeManager::instance().isDark() ? "#e6e8f2" : "#1c1d2b";

    QString html;
    html += QString("<body style='color:%1; background:transparent;'>").arg(color);

    for (const ChatMessage &msg : messages) {
        if (msg.role == "system") {
            html += QString(
                "<p style='margin:6px 0; color:%1; font-style:italic;'>%2</p>"
            ).arg(systemLabelColor(), msg.text.toHtmlEscaped());
            continue;
        }

        QString label = (msg.role == "user") ? "Вы" : "Модель";
        QString labelColor = (msg.role == "user")
            ? userLabelColor() : assistantLabelColor();

        QString content = msg.text;
        QStringList parts = content.split("```");

        html += QString(
            "<p style='margin:12px 0 4px 0; color:%1;'>"
            "<b style='color:%2;'>%3:</b></p>"
        ).arg(color, labelColor, label);

        for (int i = 0; i < parts.size(); ++i) {
            if (i % 2 == 0) {
                QString txt = parts[i].trimmed();
                if (txt.isEmpty()) continue;

                txt = txt.toHtmlEscaped();
                txt.replace("\n", "<br>");

                html += QString(
                    "<p style='margin:4px 0 12px 0; color:%1;'>%2</p>"
                ).arg(color, txt);
            } else {
                QString code = parts[i];

                int nl = code.indexOf('\n');
                if (nl > 0 && nl < 20) {
                    code = code.mid(nl + 1);
                }

                code.remove("```");

                QString escaped = code.toHtmlEscaped();

                html += QString(
                    "<table cellpadding='0' cellspacing='0' width='100%' "
                    "style='margin:8px 0 14px 0;'>"
                    "<tr><td style='background:%1; border-radius:10px; "
                    "padding:10px 14px;'>"
                    "<pre style='margin:0; white-space:pre-wrap; "
                    "font-family:Consolas,Monaco,monospace; font-size:12.5px; "
                    "color:%2;'>%3</pre>"
                    "</td></tr></table>"
                ).arg(codeBg, codeColor, escaped);
            }
        }
    }

    if (m_streaming) {
        QString escaped = m_streamingBuffer.toHtmlEscaped();
        escaped.replace("\n", "<br>");
        html += QString(
            "<p style='margin:12px 0; color:%1;'>"
            "<b style='color:%2;'>Модель:</b> %3"
            "</p>"
        ).arg(color, assistantLabelColor(), escaped);
    }

    html += "</body>";

    m_chatView->setHtml(html);
    m_chatView->moveCursor(QTextCursor::End);

    QScrollBar *sb = m_chatView->verticalScrollBar();
    sb->setValue(sb->maximum());
}

// ============================================================
// Добавление сообщений
// ============================================================

void ChatWidget::appendUserMessage(const QString &text)
{
    if (m_currentDialog < 0) return;
    m_dialogs[m_currentDialog].append({"user", text});
    renderCurrentDialog();
}

void ChatWidget::appendAssistantMessage(const QString &text)
{
    if (m_currentDialog < 0) return;
    m_dialogs[m_currentDialog].append({"assistant", text});
    renderCurrentDialog();
}

void ChatWidget::appendSystemMessage(const QString &text)
{
    if (m_currentDialog < 0) return;
    m_dialogs[m_currentDialog].append({"system", text});
    renderCurrentDialog();
}

void ChatWidget::beginAssistantMessage()
{
    if (m_currentDialog < 0) return;
    m_streaming = true;
    m_streamingBuffer.clear();
    renderCurrentDialog();
}

void ChatWidget::appendAssistantChunk(const QString &chunk)
{
    if (!m_streaming) return;
    m_streamingBuffer += chunk;
    renderCurrentDialog();
}

void ChatWidget::endAssistantMessage()
{
    if (m_currentDialog < 0) return;

    if (!m_streamingBuffer.isEmpty()) {
        m_dialogs[m_currentDialog].append({"assistant", m_streamingBuffer});
    }

    m_streamingBuffer.clear();
    m_streaming = false;
    renderCurrentDialog();
}

// ============================================================
// Копирование всего чата
// ============================================================

void ChatWidget::onCopyAllClicked()
{
    QApplication::clipboard()->setText(m_chatView->toPlainText());
    setStatusText("●  Скопировано в буфер", "#3fbf8a");
}

// ============================================================
// Прикрепление файлов
// ============================================================

void ChatWidget::onSendClicked()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty() && m_attachedFiles.isEmpty()) return;

    appendUserMessage(text.isEmpty() ? "(сообщение с файлом)" : text);
    m_input->clear();
    emit messageSent(text);
}

void ChatWidget::onAttachClicked()
{
    QStringList paths = QFileDialog::getOpenFileNames(
        this,
        "Выбрать файлы",
        QString(),
        "Поддерживаемые (*.txt *.md *.log *.json *.csv *.py *.cpp *.h *.js *.html *.css *.xml);;Все (*.*)"
    );

    if (paths.isEmpty()) return;

    for (const QString &path : paths) {
        if (!FileHandler::isSupported(path)) {
            appendSystemMessage(QString("Формат %1 не поддерживается")
                                .arg(FileHandler::extensionOf(path)));
            continue;
        }

        QString content = FileHandler::readTextFile(path);
        if (content.isEmpty()) continue;

        const int MAX_CHARS = 200 * 1024;
        if (content.size() > MAX_CHARS)
            content = content.left(MAX_CHARS) + "\n\n[... обрезано ...]";

        m_attachedFiles[path] = content;
        appendSystemMessage(QString("Прикреплён файл: %1 (%2 КБ)")
                            .arg(QFileInfo(path).fileName())
                            .arg(content.size() / 1024));
    }

    rebuildChipsRow();
}

void ChatWidget::addFileChip(const QString &path)
{
    QFileInfo info(path);
    QPushButton *chip = new QPushButton(
        QString("📄 %1   ✕").arg(info.fileName()), m_chipsRow);
    chip->setObjectName("fileChip");
    chip->setCursor(Qt::PointingHandCursor);
    chip->setToolTip(path);
    connect(chip, &QPushButton::clicked, this, [this, path](){
        removeFileChip(path);
    });
    m_chipsLayout->addWidget(chip);
}

void ChatWidget::removeFileChip(const QString &path)
{
    m_attachedFiles.remove(path);
    rebuildChipsRow();
}

void ChatWidget::rebuildChipsRow()
{
    while (QLayoutItem *item = m_chipsLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }

    for (auto it = m_attachedFiles.constBegin();
         it != m_attachedFiles.constEnd(); ++it) {
        QFileInfo info(it.key());
        QPushButton *chip = new QPushButton(
            QString("📄 %1   ✕").arg(info.fileName()), m_chipsRow);
        chip->setObjectName("fileChip");
        chip->setCursor(Qt::PointingHandCursor);
        chip->setToolTip(it.key());

        QString path = it.key();
        connect(chip, &QPushButton::clicked, this, [this, path](){
            removeFileChip(path);
        });

        m_chipsLayout->addWidget(chip);
    }

    m_chipsLayout->addStretch();
    m_chipsRow->setVisible(!m_attachedFiles.isEmpty());
}

QString ChatWidget::attachedContext() const
{
    if (m_attachedFiles.isEmpty()) return QString();

    QString result;
    for (auto it = m_attachedFiles.constBegin();
         it != m_attachedFiles.constEnd(); ++it) {
        QFileInfo info(it.key());
        result += QString("\n\n=== Файл: %1 ===\n").arg(info.fileName());
        result += it.value();
        result += "\n=== Конец файла ===\n";
    }
    return result;
}

void ChatWidget::setStatusText(const QString &text, const QString &color)
{
    m_statusLabel->setText(text);
    if (!color.isEmpty()) {
        m_statusLabel->setStyleSheet(
            QString("color: %1; font-size: 12px; font-weight: 600;").arg(color));
    }
}

void ChatWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}