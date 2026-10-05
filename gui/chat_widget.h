#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QMap>
#include <QVector>

struct ChatMessage {
    QString role;
    QString text;
};

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);

    void setCurrentDialog(int index);
    int currentDialog() const { return m_currentDialog; }

    void appendUserMessage(const QString &text);
    void appendAssistantMessage(const QString &text);
    void appendSystemMessage(const QString &text);

    void beginAssistantMessage();
    void appendAssistantChunk(const QString &chunk);
    void endAssistantMessage();

    void setStatusText(const QString &text, const QString &color = "");
    void setTitle(const QString &title);

    QString attachedContext() const;
    void refreshTheme();

signals:
    void messageSent(const QString &text);
    void toggleLeftPanelRequested();
    void toggleRightPanelRequested();

private slots:
    void onSendClicked();
    void onAttachClicked();
    void onCopyAllClicked();

private:
    void addFileChip(const QString &path);
    void removeFileChip(const QString &path);
    void rebuildChipsRow();

    void renderCurrentDialog();

    QTextEdit *m_chatView = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_attachButton = nullptr;

    QPushButton *m_toggleLeftBtn = nullptr;
    QPushButton *m_toggleRightBtn = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_statusLabel = nullptr;

    QWidget *m_chipsRow = nullptr;
    QHBoxLayout *m_chipsLayout = nullptr;

    QMap<QString, QString> m_attachedFiles;

    QMap<int, QVector<ChatMessage>> m_dialogs;
    int m_currentDialog = -1;

    QString m_streamingBuffer;
    bool m_streaming = false;
};

#endif