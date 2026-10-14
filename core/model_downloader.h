#ifndef MODEL_DOWNLOADER_H
#define MODEL_DOWNLOADER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ModelDownloader : public QObject
{
    Q_OBJECT

public:
    explicit ModelDownloader(QObject *parent = nullptr);

    void downloadModel(const QString &baseUrl, const QString &modelName);
    void cancel();
    bool isDownloading() const { return m_reply != nullptr; }

signals:
    void progress(const QString &status, int percent);
    void finished(const QString &modelName);
    void error(const QString &message);

private slots:
    void onReadyRead();
    void onFinished();

private:
    void processBuffer();

    QNetworkAccessManager *m_net = nullptr;
    QNetworkReply *m_reply = nullptr;
    QByteArray m_buffer;
    QString m_modelName;
};

#endif
