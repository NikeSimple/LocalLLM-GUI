#include "ollama_installer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>
#include <QNetworkRequest>
#include <QDebug>

OllamaInstaller::OllamaInstaller(QObject *parent)
    : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);
}

QStringList OllamaInstaller::candidatePaths()
{
    QStringList paths;
    QString localAppData = qgetenv("LOCALAPPDATA");
    if (!localAppData.isEmpty()) {
        paths << localAppData + "/Programs/Ollama/ollama.exe";
        paths << localAppData + "/Ollama/ollama.exe";
    }
    QString programFiles = qgetenv("ProgramFiles");
    if (!programFiles.isEmpty()) {
        paths << programFiles + "/Ollama/ollama.exe";
    }
    QString programFilesX86 = qgetenv("ProgramFiles(x86)");
    if (!programFilesX86.isEmpty()) {
        paths << programFilesX86 + "/Ollama/ollama.exe";
    }
    return paths;
}

bool OllamaInstaller::isOllamaInstalled() const
{
    return !ollamaExecutablePath().isEmpty();
}

QString OllamaInstaller::ollamaExecutablePath() const
{
    for (const QString &path : candidatePaths()) {
        if (QFile::exists(path)) return path;
    }
    return QString();
}

void OllamaInstaller::downloadInstaller()
{
    if (m_reply) {
        emit errorOccurred("Скачивание уже идёт");
        return;
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) tempDir = QDir::tempPath();
    m_downloadPath = tempDir + "/OllamaSetup.exe";

    if (QFile::exists(m_downloadPath)) {
        QFile::remove(m_downloadPath);
    }

    QUrl url("https://ollama.com/download/OllamaSetup.exe");
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setHeader(QNetworkRequest::UserAgentHeader, "LocalLLM-GUI/1.0");

    m_reply = m_net->get(req);
    m_downloadBuffer.clear();

    connect(m_reply, &QNetworkReply::readyRead,
            this, &OllamaInstaller::onDownloadReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &OllamaInstaller::downloadProgress);
    connect(m_reply, &QNetworkReply::finished,
            this, &OllamaInstaller::onDownloadFinished);
}

void OllamaInstaller::onDownloadReadyRead()
{
    if (!m_reply) return;
    QByteArray chunk = m_reply->readAll();
    QFile file(m_downloadPath);
    if (file.open(QIODevice::Append)) {
        file.write(chunk);
        file.close();
    } else {
        emit errorOccurred("Не удалось записать файл: " + m_downloadPath);
        cancelDownload();
    }
}

void OllamaInstaller::onDownloadFinished()
{
    if (!m_reply) return;

    QNetworkReply::NetworkError err = m_reply->error();
    QString errStr = m_reply->errorString();
    m_reply->deleteLater();
    m_reply = nullptr;

    if (err != QNetworkReply::NoError) {
        emit errorOccurred("Ошибка скачивания: " + errStr);
        return;
    }

    if (!QFile::exists(m_downloadPath) ||
        QFileInfo(m_downloadPath).size() < 1024) {
        emit errorOccurred("Скачанный файл повреждён или пуст");
        return;
    }

    emit downloadFinished(m_downloadPath);
}

void OllamaInstaller::cancelDownload()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (QFile::exists(m_downloadPath)) {
        QFile::remove(m_downloadPath);
    }
}

bool OllamaInstaller::runSilentInstall(const QString &installerPath)
{
    if (!QFile::exists(installerPath)) {
        emit errorOccurred("Установщик не найден: " + installerPath);
        return false;
    }

    if (m_installProcess) {
        emit errorOccurred("Установка уже идёт");
        return false;
    }

    m_installProcess = new QProcess(this);
    m_installProcess->setProgram(installerPath);

    QStringList args;
    args << "/VERYSILENT"
         << "/SUPPRESSMSGBOXES"
         << "/NORESTART"
         << "/SP-";
    m_installProcess->setArguments(args);

    connect(m_installProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &OllamaInstaller::onInstallProcessFinished);

    m_installProcess->start();
    if (!m_installProcess->waitForStarted(10000)) {
        emit errorOccurred("Не удалось запустить установщик");
        delete m_installProcess;
        m_installProcess = nullptr;
        return false;
    }

    return true;
}

void OllamaInstaller::onInstallProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    if (m_installProcess) {
        m_installProcess->deleteLater();
        m_installProcess = nullptr;
    }

    bool ok = (status == QProcess::NormalExit) && (exitCode == 0);
    if (!isOllamaInstalled()) ok = false;

    emit installFinished(ok);
}

bool OllamaInstaller::startOllamaServe()
{
    QString exe = ollamaExecutablePath();
    if (exe.isEmpty()) {
        emit errorOccurred("Ollama не найдена для запуска");
        return false;
    }

    if (m_serveProcess) {
        return true;
    }

    m_serveProcess = new QProcess(this);
    m_serveProcess->setProgram(exe);
    m_serveProcess->setArguments({"serve"});
    m_serveProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_serveProcess->startDetached();

    return true;
}
