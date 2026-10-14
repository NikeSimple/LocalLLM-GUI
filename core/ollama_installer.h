#ifndef OLLAMA_INSTALLER_H
#define OLLAMA_INSTALLER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>

class OllamaInstaller : public QObject
{
    Q_OBJECT

public:
    explicit OllamaInstaller(QObject *parent = nullptr);

    bool isOllamaInstalled() const;
    QString ollamaExecutablePath() const;
    void downloadInstaller();
    bool runSilentInstall(const QString &installerPath);
    void cancelDownload();
    bool startOllamaServe();

signals:
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(const QString &installerPath);
    void errorOccurred(const QString &error);
    void installFinished(bool success);

private slots:
    void onDownloadReadyRead();
    void onDownloadFinished();
    void onInstallProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    static QStringList candidatePaths();

    QNetworkAccessManager *m_net = nullptr;
    QNetworkReply *m_reply = nullptr;
    QProcess *m_installProcess = nullptr;
    QProcess *m_serveProcess = nullptr;

    QString m_downloadPath;
    QByteArray m_downloadBuffer;
};

#endif
