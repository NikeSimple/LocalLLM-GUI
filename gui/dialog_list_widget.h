#ifndef DIALOG_LIST_WIDGET_H
#define DIALOG_LIST_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>

class DialogListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DialogListWidget(QWidget *parent = nullptr);

    int currentRow() const;
    int count() const;
    void setCurrentRow(int row);
    void updateDialogTitle(int index, const QString &newTitle);
    void updateDialogPreview(int index, const QString &preview);
    void createNewDialog();
    void addDialogCardStatic(const QString &title,
                             const QString &preview,
                             const QString &time);

    void setUserName(const QString &name);
    void clearAllDialogs();

signals:
    void newDialogRequested();
    void dialogSelected(int dialogId);
    void settingsRequested();
    void themeToggleRequested();

private slots:
    void onNewDialogClicked();
    void onItemClicked();
    void onSearchChanged(const QString &text);

private:
    void setupNewDialogButton(QVBoxLayout *layout);
    void setupSearch(QVBoxLayout *layout);
    void setupGroupHeader(QVBoxLayout *layout, const QString &title);
    void setupDialogsList(QVBoxLayout *layout);
    void setupProfileFooter(QVBoxLayout *layout);
    void addDialogCard(const QString &title, const QString &preview, const QString &time);

    QPushButton *m_newDialogButton = nullptr;
    QLineEdit *m_searchField = nullptr;
    QListWidget *m_list = nullptr;

    QLabel *m_avatar = nullptr;
    QLabel *m_userName = nullptr;
    QLabel *m_userStatus = nullptr;
    QPushButton *m_themeButton = nullptr;
    QPushButton *m_settingsButton = nullptr;
};

#endif