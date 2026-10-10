#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QPropertyAnimation>
#include <QMap>
#include "chat_widget.h"
#include "dialog_list_widget.h"
#include "settings_panel.h"
#include "settings_overlay.h"
#include "../core/api_client.h"
#include "../db/database_manager.h"
#include "../db/dialog_repository.h"
#include "../db/message_repository.h"
#include "../db/prompt_repository.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMessageSent(const QString &text);
    void onMessageAdded(int dialogIndex, const QString &role, const QString &text);
    void onPong(bool ok, int count);
    void onModelsReceived(const QStringList &models);
    void onChatChunk(const QString &chunk);
    void onChatFinished();
    void onTitleGenerated(const QString &title);
    void onDialogSelected(int row);

    void toggleLeftPanel();
    void toggleRightPanel();
    void toggleSettingsOverlay();

private:
    void loadDialogsFromDb();
    void exportDialogByRow(int row);

    DialogListWidget *m_dialogList = nullptr;
    ChatWidget *m_chat = nullptr;
    SettingsPanel *m_settings = nullptr;
    SettingsOverlay *m_settingsOverlay = nullptr;
    ApiClient *m_api = nullptr;
    DatabaseManager *m_db = nullptr;
    DialogRepository *m_dialogRepo = nullptr;
    MessageRepository *m_messageRepo = nullptr;
    PromptRepository *m_promptRepo = nullptr;

    QPropertyAnimation *m_leftAnim = nullptr;
    QPropertyAnimation *m_rightAnim = nullptr;
    QPropertyAnimation *m_settingsAnim = nullptr;

    bool m_leftCollapsed = false;
    bool m_rightCollapsed = false;
    bool m_settingsOpen = false;

    int m_leftFullWidth = 300;
    int m_rightFullWidth = 340;
    int m_settingsOverlayHeight = 420;

    QMap<int, int> m_dialogIndexToDbId;
};

#endif