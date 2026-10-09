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
#include <QRegularExpression>
#include <QIcon>
#include <QDebug>
#include "theme_manager.h"
#include "../core/file_handler.h"

// ============================================================
// Рендер markdown
// ============================================================

static QString renderTextBlock(const QString &text, const QString &color)
{
    if (text.trimmed().isEmpty()) return QString();
    QString txt = text.toHtmlEscaped();
    txt.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
    txt.replace(QRegularExpression("(?<!\\*)\\*([^*]+?)\\*(?!\\*)"), "<i>\\1</i>");
    txt.replace(QRegularExpression("(?m)^- (.+)$"), "• \\1");
    txt.replace("\n", "<br>");
    return QString("<div style='margin:6px 0 12px 0; color:%1; line-height:1.5;'>%2</div>")
        .arg(color, txt);
}

static QString renderTable(const QStringList &rows, const QString &color,
                            const QString &border, const QString &headerBg,
                            const QString &rowBg1, const QString &rowBg2)
{
    if (rows.size() < 2) return QString();
    QVector<QStringList> cells;
    for (const QString &row : rows) {
        QString cleaned = row.trimmed();
        if (cleaned.startsWith("|")) cleaned = cleaned.mid(1);
        if (cleaned.endsWith("|")) cleaned.chop(1);
        QStringList parts = cleaned.split("|");
        for (QString &p : parts) p = p.trimmed();
        cells.append(parts);
    }
    if (cells.isEmpty()) return QString();

    QStringList header = cells[0];
    int dataStart = 1;
    if (cells.size() > 1) {
        bool isSep = true;
        for (const QString &c : cells[1]) {
            if (!c.isEmpty() && !c.contains(QRegularExpression("^[-: ]+$"))) {
                isSep = false; break;
            }
        }
        if (isSep) dataStart = 2;
    }

    QString html = "<table cellpadding='0' cellspacing='0' style='margin:10px 0 14px 0; "
                   "border-collapse:separate; border-spacing:0; width:100%; "
                   "border:1px solid " + border + "; border-radius:10px; overflow:hidden;'>";
    html += "<tr>";
    for (const QString &h : header) {
        QString cell = h.toHtmlEscaped();
        cell.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
        html += QString("<th style='background:%1; color:%2; padding:10px 14px; "
                        "text-align:left; font-weight:700; font-size:13px; "
                        "border-bottom:1px solid %3;'>%4</th>")
                    .arg(headerBg, color, border, cell);
    }
    html += "</tr>";
    for (int i = dataStart; i < cells.size(); ++i) {
        QString rowBg = ((i - dataStart) % 2 == 0) ? rowBg1 : rowBg2;
        html += QString("<tr style='background:%1;'>").arg(rowBg);
        const QStringList &row = cells[i];
        for (int j = 0; j < header.size(); ++j) {
            QString cell = (j < row.size()) ? row[j] : "";
            cell = cell.toHtmlEscaped();
            cell.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
            html += QString("<td style='padding:8px 14px; color:%1; font-size:13px; "
                            "border-bottom:1px solid %2;'>%3</td>")
                        .arg(color, border, cell);
        }
        html += "</tr>";
    }
    html += "</table>";
    return html;
}

static QString renderTextAndTables(const QString &text, const QString &color,
                                    const QString &border, const QString &headerBg,
                                    const QString &rowBg1, const QString &rowBg2)
{
    QStringList lines = text.split("\n");
    QString html, textBuffer;
    QStringList tableRows;

    auto flushText = [&](){
        if (!textBuffer.isEmpty()) {
            html += renderTextBlock(textBuffer, color);
            textBuffer.clear();
        }
    };
    auto flushTable = [&](){
        if (!tableRows.isEmpty()) {
            html += renderTable(tableRows, color, border, headerBg, rowBg1, rowBg2);
            tableRows.clear();
        }
    };

    for (const QString &line : lines) {
        QString t = line.trimmed();
        bool isTable = t.startsWith("|") && t.endsWith("|") && t.count("|") >= 2;
        if (isTable) {
            flushText();
            tableRows.append(line);
        } else {
            flushTable();
            if (!textBuffer.isEmpty()) textBuffer += "\n";
            textBuffer += line;
        }
    }
    flushTable();
    flushText();
    return html;
}

static bool blockContainsTable(const QString &block)
{
    int n = 0;
    for (const QString &l : block.split("\n")) {
        QString t = l.trimmed();
        if (t.startsWith("|") && t.endsWith("|") && t.count("|") >= 2) n++;
    }
    return n >= 2;
}

// ============================================================
// Конструктор
// ============================================================

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    QWidget *toolbar = new QWidget(this);
    toolbar->setObjectName("chatToolbar");
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(6, 6, 6, 6);
    toolbarLayout->setSpacing(6);

    m_toggleLeftBtn = new QPushButton(toolbar);
    m_toggleLeftBtn->setObjectName("iconButton");
    m_toggleLeftBtn->setIcon(QIcon(":/icons/menu.svg"));
    m_toggleLeftBtn->setIconSize(QSize(20, 20));
    m_toggleLeftBtn->setFixedSize(36, 36);
    m_toggleLeftBtn->setCursor(Qt::PointingHandCursor);
    connect(m_toggleLeftBtn, &QPushButton::clicked,
            this, &ChatWidget::toggleLeftPanelRequested);

    m_titleLabel = new QLabel("Новый диалог", toolbar);
    m_titleLabel->setObjectName("chatTitle");

    m_statusLabel = new QLabel("●  Не подключено", toolbar);
    m_statusLabel->setObjectName("chatStatus");

    m_exportBtn = new QPushButton(toolbar);
    m_exportBtn->setObjectName("iconButton");
    m_exportBtn->setIcon(QIcon(":/icons/download.svg"));
    m_exportBtn->setIconSize(QSize(20, 20));
    m_exportBtn->setFixedSize(36, 36);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setToolTip("Экспортировать диалог в JSON");
    connect(m_exportBtn, &QPushButton::clicked,
            this, &ChatWidget::onExportClicked);

    m_copyAllBtn = new QPushButton(toolbar);
    m_copyAllBtn->setObjectName("iconButton");
    m_copyAllBtn->setIcon(QIcon(":/icons/copy.svg"));
    m_copyAllBtn->setIconSize(QSize(20, 20));
    m_copyAllBtn->setFixedSize(36, 36);
    m_copyAllBtn->setCursor(Qt::PointingHandCursor);
    m_copyAllBtn->setToolTip("Скопировать весь чат");
    connect(m_copyAllBtn, &QPushButton::clicked,
            this, &ChatWidget::onCopyAllClicked);

    m_toggleRightBtn = new QPushButton(toolbar);
    m_toggleRightBtn->setObjectName("iconButton");
    m_toggleRightBtn->setIcon(QIcon(":/icons/menu.svg"));
    m_toggleRightBtn->setIconSize(QSize(20, 20));
    m_toggleRightBtn->setFixedSize(36, 36);
    m_toggleRightBtn->setCursor(Qt::PointingHandCursor);
    connect(m_toggleRightBtn, &QPushButton::clicked,
            this, &ChatWidget::toggleRightPanelRequested);

    toolbarLayout->addWidget(m_toggleLeftBtn);
    toolbarLayout->addSpacing(4);
    toolbarLayout->addWidget(m_titleLabel);
    toolbarLayout->addSpacing(8);
    toolbarLayout->addWidget(m_statusLabel);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_exportBtn);
    toolbarLayout->addWidget(m_copyAllBtn);
    toolbarLayout->addSpacing(4);
    toolbarLayout->addWidget(m_toggleRightBtn);

    layout->addWidget(toolbar);

    QFrame *line = new QFrame(this);
    line->setObjectName("toolbarSeparator");
    line->setFrameShape(QFrame::HLine);
    layout->addWidget(line);

    m_chatView = new QTextEdit(this);
    m_chatView->setReadOnly(true);
    m_chatView->setPlaceholderText("История диалога появится здесь...");
    m_chatView->setObjectName("chatView");
    m_chatView->setAcceptRichText(true);
    layout->addWidget(m_chatView, 1);

    m_chipsRow = new QWidget(this);
    m_chipsRow->setObjectName("chipsRow");
    m_chipsLayout = new QHBoxLayout(m_chipsRow);
    m_chipsLayout->setContentsMargins(0, 0, 0, 0);
    m_chipsLayout->setSpacing(6);
    m_chipsRow->setVisible(false);
    layout->addWidget(m_chipsRow);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(8);

    m_attachButton = new QPushButton(this);
    m_attachButton->setObjectName("attachButton");
    m_attachButton->setIcon(QIcon(":/icons/attach.svg"));
    m_attachButton->setIconSize(QSize(20, 20));
    m_attachButton->setFixedSize(40, 40);
    m_attachButton->setCursor(Qt::PointingHandCursor);
    m_attachButton->setToolTip("Прикрепить файл");
    connect(m_attachButton, &QPushButton::clicked,
            this, &ChatWidget::onAttachClicked);
    inputLayout->addWidget(m_attachButton);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Введите сообщение... (Enter — отправить)");
    connect(m_input, &QLineEdit::returnPressed,
            this, &ChatWidget::onSendClicked);
    connect(m_input, &QLineEdit::textChanged,
            this, &ChatWidget::onInputChanged);
    inputLayout->addWidget(m_input);

    m_sendButton = new QPushButton("Отправить", this);
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setIcon(QIcon(":/icons/send.svg"));
    m_sendButton->setIconSize(QSize(18, 18));
    connect(m_sendButton, &QPushButton::clicked,
            this, &ChatWidget::onSendClicked);
    inputLayout->addWidget(m_sendButton);

    layout->addLayout(inputLayout);

    updateSendButtonState();
}

ChatWidget::~ChatWidget() = default;

// ============================================================
// Вспомогательные цвета
// ============================================================

static QString bodyColor()
{
    return ThemeManager::instance().isDark() ? "#f2f2f7" : "#1c1d2b";
}
static QString userLabelColor() { return "#3f6fd4"; }
static QString assistantLabelColor() { return "#6a97f2"; }
static QString systemLabelColor() { return "#a0a2b4"; }

// ============================================================
// Публичные методы
// ============================================================

void ChatWidget::refreshTheme() { renderCurrentDialog(); }

void ChatWidget::setBusy(bool busy)
{
    m_busy = busy;
    updateSendButtonState();
}

void ChatWidget::updateSendButtonState()
{
    bool hasText = !m_input->text().trimmed().isEmpty() || !m_attachedFiles.isEmpty();
    m_sendButton->setEnabled(!m_busy && hasText);
}

void ChatWidget::onInputChanged(const QString &) { updateSendButtonState(); }

void ChatWidget::setCurrentDialog(int index)
{
    if (!m_dialogs.contains(index)) m_dialogs[index] = QVector<ChatMessage>();
    m_currentDialog = index;
    renderCurrentDialog();
}

void ChatWidget::loadMessages(int dialogId, const QVector<ChatMessage> &messages)
{
    m_dialogs[dialogId] = messages;
    m_currentDialog = dialogId;
    renderCurrentDialog();
}

void ChatWidget::clear()
{
    m_dialogs.clear();
    m_currentDialog = -1;
    m_streamingBuffer.clear();
    m_streaming = false;
    m_chatView->clear();
    m_attachedFiles.clear();
    rebuildChipsRow();
    setTitle("Новый диалог");
    updateSendButtonState();
}

void ChatWidget::clearAttachments()
{
    m_attachedFiles.clear();
    rebuildChipsRow();
    updateSendButtonState();
}

QString ChatWidget::lastAssistantText() const
{
    if (m_currentDialog < 0) return QString();
    const QVector<ChatMessage> &msgs = m_dialogs.value(m_currentDialog);
    for (int i = msgs.size() - 1; i >= 0; --i) {
        if (msgs[i].role == "assistant") return msgs[i].text;
    }
    return QString();
}

// ============================================================
// Рендер чата
// ============================================================

void ChatWidget::renderCurrentDialog()
{
    if (m_currentDialog < 0) { m_chatView->clear(); return; }
    const QVector<ChatMessage> &messages = m_dialogs.value(m_currentDialog);
    if (messages.isEmpty() && !m_streaming) { m_chatView->clear(); return; }

    QString color = bodyColor();
    QString codeBg = ThemeManager::instance().isDark() ? "#1b1c23" : "#f4f6fb";
    QString codeColor = ThemeManager::instance().isDark() ? "#e6e8f2" : "#1c1d2b";
    QString border = ThemeManager::instance().isDark()
        ? "rgba(255,255,255,0.15)" : "rgba(60,70,110,0.15)";
    QString fileBg = ThemeManager::instance().isDark()
        ? "rgba(123,168,255,0.15)" : "rgba(123,168,255,0.1)";
    QString tableHeaderBg = ThemeManager::instance().isDark()
        ? "rgba(123,168,255,0.22)" : "rgba(123,168,255,0.18)";
    QString tableRowBg1 = ThemeManager::instance().isDark()
        ? "rgba(255,255,255,0.03)" : "rgba(255,255,255,0.4)";
    QString tableRowBg2 = "transparent";

    QString html;
    html += QString("<body style='color:%1; background:transparent; font-size:14px;'>").arg(color);

    for (const ChatMessage &msg : messages) {
        if (msg.role == "system") {
            html += QString("<p style='margin:6px 0; color:%1; font-style:italic;'>%2</p>")
                        .arg(systemLabelColor(), msg.text.toHtmlEscaped());
            continue;
        }

        QString label = (msg.role == "user") ? "Вы" : "Модель";
        QString labelColor = (msg.role == "user") ? userLabelColor() : assistantLabelColor();

        html += QString("<div style='margin:14px 0 4px 0; color:%1;'>"
                        "<b style='color:%2; font-size:14px;'>%3:</b></div>")
                    .arg(color, labelColor, label);

        if (!msg.files.isEmpty()) {
            html += "<div style='margin:6px 0 8px 0;'>";
            for (const QString &f : msg.files) {
                QString fileName = QFileInfo(f).fileName();
                html += QString("<span style='display:inline-block; background:%1; "
                                "border:1px solid %2; border-radius:10px; "
                                "padding:6px 10px; margin:2px 4px 2px 0; "
                                "font-size:12px; color:%3;'>📎 %4</span>")
                            .arg(fileBg, border, color, fileName.toHtmlEscaped());
            }
            html += "</div>";
        }

        QString content = msg.text;
        QStringList codeParts = content.split("```");

        for (int i = 0; i < codeParts.size(); ++i) {
            if (i % 2 == 0) {
                html += renderTextAndTables(codeParts[i], color, border,
                                             tableHeaderBg, tableRowBg1, tableRowBg2);
            } else {
                QString block = codeParts[i];
                int nl = block.indexOf('\n');
                QString lang;
                if (nl > 0 && nl < 20) {
                    lang = block.left(nl).trimmed().toLower();
                    block = block.mid(nl + 1);
                }
                block.remove("```");

                bool isTable = blockContainsTable(block);
                bool isMarkdown = (lang == "markdown" || lang == "md");

                if (isTable || isMarkdown) {
                    html += renderTextAndTables(block, color, border,
                                                 tableHeaderBg, tableRowBg1, tableRowBg2);
                } else {
                    QString escaped = block.toHtmlEscaped();
                    html += QString("<table cellpadding='0' cellspacing='0' width='100%' "
                                    "style='margin:8px 0 14px 0;'>"
                                    "<tr><td style='background:%1; border:1px solid %4; "
                                    "border-radius:10px; padding:12px 14px;'>"
                                    "<pre style='margin:0; white-space:pre-wrap; "
                                    "font-family:Consolas,monospace; font-size:12.5px; color:%2;'>%3</pre>"
                                    "</td></tr></table>")
                                .arg(codeBg, codeColor, escaped, border);
                }
            }
        }
    }

    if (m_streaming) {
        QString escaped = m_streamingBuffer.toHtmlEscaped();
        escaped.replace("\n", "<br>");
        html += QString("<div style='margin:12px 0; color:%1;'>"
                        "<b style='color:%2;'>Модель:</b> %3</div>")
                    .arg(color, assistantLabelColor(), escaped);
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
    ChatMessage msg;
    msg.role = "user";
    msg.text = text;
    for (auto it = m_attachedFiles.constBegin(); it != m_attachedFiles.constEnd(); ++it) {
        msg.files.append(it.key());
    }
    m_dialogs[m_currentDialog].append(msg);
    emit messageAdded(m_currentDialog, "user", text);
    renderCurrentDialog();
}

void ChatWidget::appendAssistantMessage(const QString &text)
{
    if (m_currentDialog < 0) return;
    ChatMessage msg;
    msg.role = "assistant";
    msg.text = text;
    m_dialogs[m_currentDialog].append(msg);
    emit messageAdded(m_currentDialog, "assistant", text);
    renderCurrentDialog();
}

void ChatWidget::appendSystemMessage(const QString &text)
{
    if (m_currentDialog < 0) return;
    ChatMessage msg;
    msg.role = "system";
    msg.text = text;
    m_dialogs[m_currentDialog].append(msg);
    renderCurrentDialog();
}

void ChatWidget::beginAssistantMessage()
{
    if (m_currentDialog < 0) return;
    m_streaming = true;
    m_streamingBuffer.clear();
    setBusy(true);
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
        ChatMessage msg;
        msg.role = "assistant";
        msg.text = m_streamingBuffer;
        m_dialogs[m_currentDialog].append(msg);
        emit messageAdded(m_currentDialog, "assistant", m_streamingBuffer);
    }
    m_streamingBuffer.clear();
    m_streaming = false;
    setBusy(false);
    renderCurrentDialog();
}

// ============================================================
// Кнопки тулбара
// ============================================================

void ChatWidget::onCopyAllClicked()
{
    QApplication::clipboard()->setText(m_chatView->toPlainText());
    setStatusText("●  Скопировано в буфер", "#3fbf8a");
}

void ChatWidget::onExportClicked()
{
    if (m_currentDialog < 0) {
        setStatusText("●  Нет открытого диалога", "#ef6a7a");
        return;
    }
    emit exportDialogRequested(m_currentDialog);
}

// ============================================================
// Отправка и файлы
// ============================================================

void ChatWidget::onSendClicked()
{
    if (m_busy) return;
    QString text = m_input->text().trimmed();
    if (text.isEmpty() && m_attachedFiles.isEmpty()) return;
    appendUserMessage(text.isEmpty() ? "(сообщение с файлом)" : text);
    m_input->clear();
    emit messageSent(text);
    updateSendButtonState();
}

void ChatWidget::onAttachClicked()
{
    if (m_busy) return;
    QStringList paths = QFileDialog::getOpenFileNames(this, "Файлы", QString(),
        "Все (*.*);;Текст (*.txt *.md *.log *.json *.csv *.py *.cpp *.h *.js *.html *.css *.xml)");
    if (paths.isEmpty()) return;
    for (const QString &path : paths) {
        if (!FileHandler::isSupported(path)) continue;
        QString content = FileHandler::readTextFile(path);
        if (content.isEmpty()) continue;
        const int MAX_CHARS = 200 * 1024;
        if (content.size() > MAX_CHARS) content = content.left(MAX_CHARS) + "\n[... обрезано ...]";
        m_attachedFiles[path] = content;
    }
    rebuildChipsRow();
    updateSendButtonState();
}

void ChatWidget::removeFileChip(const QString &path)
{
    m_attachedFiles.remove(path);
    rebuildChipsRow();
    updateSendButtonState();
}

void ChatWidget::rebuildChipsRow()
{
    while (QLayoutItem *item = m_chipsLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
    for (auto it = m_attachedFiles.constBegin(); it != m_attachedFiles.constEnd(); ++it) {
        QFileInfo info(it.key());
        QPushButton *chip = new QPushButton(
            QString("%1   ✕").arg(info.fileName()), m_chipsRow);
        chip->setObjectName("fileChip");
        chip->setCursor(Qt::PointingHandCursor);
        chip->setToolTip(it.key());
        QString path = it.key();
        connect(chip, &QPushButton::clicked, this, [this, path](){ removeFileChip(path); });
        m_chipsLayout->addWidget(chip);
    }
    m_chipsLayout->addStretch();
    m_chipsRow->setVisible(!m_attachedFiles.isEmpty());
}

QString ChatWidget::attachedContext() const
{
    if (m_attachedFiles.isEmpty()) return QString();
    QString result;
    for (auto it = m_attachedFiles.constBegin(); it != m_attachedFiles.constEnd(); ++it) {
        result += QString("\n\n=== Файл: %1 ===\n").arg(QFileInfo(it.key()).fileName());
        result += it.value();
        result += "\n=== Конец ===\n";
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