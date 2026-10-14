#ifndef SETUP_WIZARD_H
#define SETUP_WIZARD_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include "../core/ollama_installer.h"
#include "../core/model_downloader.h"

class SetupWizard : public QDialog
{
    Q_OBJECT

public:
    explicit SetupWizard(QWidget *parent = nullptr);

signals:
    void setupCompleted();

private slots:
    void onStep1InstallClicked();
    void onStep1SkipClicked();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished(const QString &path);
    void onInstallFinished(bool success);
    void onStep2DownloadClicked();
    void onStep2SkipClicked();
    void onModelProgress(const QString &status, int percent);
    void onModelFinished(const QString &modelName);
    void onError(const QString &message);

private:
    void buildStep1();
    void buildStep2();
    void buildStep3();

    QStackedWidget *m_stack = nullptr;

    QLabel *m_step1Status = nullptr;
    QProgressBar *m_step1Progress = nullptr;
    QPushButton *m_step1InstallBtn = nullptr;
    QPushButton *m_step1SkipBtn = nullptr;

    QLabel *m_step2Status = nullptr;
    QProgressBar *m_step2Progress = nullptr;
    QPushButton *m_step2DownloadBtn = nullptr;
    QPushButton *m_step2SkipBtn = nullptr;

    QLabel *m_step3Label = nullptr;
    QPushButton *m_step3FinishBtn = nullptr;

    OllamaInstaller *m_installer = nullptr;
    ModelDownloader *m_downloader = nullptr;

    QString m_currentModel = "llama3.1:8b";
};

#endif
