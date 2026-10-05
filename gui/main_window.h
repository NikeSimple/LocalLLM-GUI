#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QPropertyAnimation>
#include "chat_widget.h"
#include "dialog_list_widget.h"
#include "settings_panel.h"
#include "../core/api_client.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMessageSent(const QString &text);
    void onPong(bool ok, int count);
    void onModelsReceived(const QStringList &models);
    void onChatChunk(const QString &chunk);
    void onChatFinished();

    void toggleLeftPanel();
    void toggleRightPanel();
    void openSettingsDialog();

private:
    DialogListWidget *m_dialogList = nullptr;
    ChatWidget *m_chat = nullptr;
    SettingsPanel *m_settings = nullptr;
    ApiClient *m_api = nullptr;

    QPropertyAnimation *m_leftAnim = nullptr;
    QPropertyAnimation *m_rightAnim = nullptr;

    bool m_leftCollapsed = false;
    bool m_rightCollapsed = false;

    int m_leftFullWidth = 300;
    int m_rightFullWidth = 340;
};

#endif