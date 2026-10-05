#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QTabWidget>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    QWidget *buildAccountTab();
    QWidget *buildAppearanceTab();
    QWidget *buildServerTab();
    QWidget *buildDataTab();

    QTabWidget *m_tabs = nullptr;
};

#endif