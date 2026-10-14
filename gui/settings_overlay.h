#ifndef SETTINGS_OVERLAY_H
#define SETTINGS_OVERLAY_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include "../db/dialog_repository.h"
#include "../db/message_repository.h"
#include "../db/prompt_repository.h"

class SettingsOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsOverlay(QWidget *parent = nullptr);

    void setRepositories(DialogRepository *dialogRepo,
                         MessageRepository *messageRepo);

    void setPromptRepository(PromptRepository *promptRepo);

signals:
    void closeRequested();
    void profileNameChanged(const QString &newName);
    void clearHistoryRequested();
    void promptApplied(const QString &text);

private:
    QWidget *buildAccountTab();
    QWidget *buildAppearanceTab();
    QWidget *buildServerTab();
    QWidget *buildDataTab();
    QWidget *buildTemplatesTab();

    void onSaveNameClicked();
    void onExportClicked();
    void onClearHistoryClicked();
    void onCheckConnectionClicked();

    void refreshTemplatesList();
    void onCreateTemplateClicked();
    void onEditTemplateClicked();
    void onDeleteTemplateClicked();
    void onApplyTemplateClicked();
    void onTemplateCheckChanged(int id, bool checked);
    void collectAndEmitActivePrompts();

    QTabWidget *m_tabs = nullptr;

    QLineEdit *m_nameEdit = nullptr;
    QLabel *m_nameStatus = nullptr;
    QLabel *m_statsLabel = nullptr;

    QLineEdit *m_urlEdit = nullptr;
    QLabel *m_serverStatus = nullptr;

    QListWidget *m_templatesList = nullptr;

    DialogRepository *m_dialogRepo = nullptr;
    MessageRepository *m_messageRepo = nullptr;
    PromptRepository *m_promptRepo = nullptr;
};

#endif