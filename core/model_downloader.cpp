#include "model_downloader.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ModelDownloader::ModelDownloader(QObject *parent)
    : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);
}

void ModelDownloader::downloadModel(const QString &baseUrl, const QString &modelName)
{
    if (m_reply) {
        emit error("Скачивание уже идёт");
        return;
    }

    m_modelName = modelName;
    m_buffer.clear();

    QUrl url(baseUrl + "/api/pull");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["name"] = modelName;
    body["stream"] = true;

    m_reply = m_net->post(req, QJsonDocument(body).toJson());

    connect(m_reply, &QNetworkReply::readyRead,
            this, &ModelDownloader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished,
            this, &ModelDownloader::onFinished);
}

void ModelDownloader::onReadyRead()
{
    if (!m_reply) return;
    m_buffer.append(m_reply->readAll());
    processBuffer();
}

void ModelDownloader::processBuffer()
{
    while (true) {
        int nl = m_buffer.indexOf('\n');
        if (nl < 0) break;

        QByteArray line = m_buffer.left(nl).trimmed();
        m_buffer.remove(0, nl + 1);

        if (line.isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) continue;

        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            emit error(obj.value("error").toString());
            cancel();
            return;
        }

        QString status = obj.value("status").toString();
        int percent = -1;

        if (obj.contains("completed") && obj.contains("total")) {
            qint64 completed = obj.value("completed").toVariant().toLongLong();
            qint64 total = obj.value("total").toVariant().toLongLong();
            if (total > 0) {
                percent = static_cast<int>((completed * 100) / total);
            }
        }

        emit progress(status, percent);
    }
}

void ModelDownloader::onFinished()
{
    if (!m_reply) return;

    QNetworkReply::NetworkError err = m_reply->error();
    QString errStr = m_reply->errorString();
    m_reply->deleteLater();
    m_reply = nullptr;

    if (err != QNetworkReply::NoError) {
        emit error("Ошибка скачивания модели: " + errStr);
        return;
    }

    emit finished(m_modelName);
}

void ModelDownloader::cancel()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}
