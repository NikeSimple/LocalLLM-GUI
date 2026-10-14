#include "api_client.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QDebug>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);
}

void ApiClient::ping()
{
    QUrl url(m_baseUrl + "/api/tags");
    QNetworkRequest req(url);
    QNetworkReply *reply = m_net->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit pongReceived(false, 0);
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray arr = doc.object().value("models").toArray();
        emit pongReceived(true, arr.size());
    });
}

void ApiClient::listModels()
{
    QUrl url(m_baseUrl + "/api/tags");
    QNetworkRequest req(url);
    QNetworkReply *reply = m_net->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit modelsReceived({});
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray arr = doc.object().value("models").toArray();

        QStringList names;
        for (const auto &v : arr) {
            names << v.toObject().value("name").toString();
        }
        emit modelsReceived(names);
    });
}

void ApiClient::generateTitle(const QString &model, const QString &userText)
{
    QUrl url(m_baseUrl + "/api/chat");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setTransferTimeout(20000);

    QJsonArray messages;

    QJsonObject sys;
    sys["role"] = "system";
    sys["content"] =
        "Ты — генератор названий диалогов. По сообщению пользователя "
        "придумай название РОВНО из 2 слов на русском языке. "
        "Не больше и не меньше. Без кавычек, без точки, без пояснений.";
    messages.append(sys);

    QJsonObject user;
    user["role"] = "user";
    user["content"] = userText.left(500);
    messages.append(user);

    QJsonObject body;
    body["model"] = model;
    body["messages"] = messages;
    body["stream"] = false;

    QJsonObject options;
    options["temperature"] = 0.3;
    options["num_predict"] = 20;
    body["options"] = options;

    QNetworkReply *reply = m_net->post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit titleGenerated(QString());
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        QString title = obj.value("message").toObject()
                           .value("content").toString().trimmed();

        title.remove(QRegularExpression("^[\"'«»\\s]+"));
        title.remove(QRegularExpression("[\"'«».\\s]+$"));
        title.remove(QRegularExpression(
            "^(Название|Заголовок|Title)\\s*:\\s*",
            QRegularExpression::CaseInsensitiveOption));

        if (title.length() > 60) title = title.left(60);
        if (title.isEmpty()) title = "Новый диалог";

        emit titleGenerated(title);
    });
}

void ApiClient::sendChat(const QString &model,
                         const QJsonArray &messages,
                         const QJsonObject &options)
{
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }

    m_buffer.clear();
    m_finishedEmitted = false;

    QUrl url(m_baseUrl + "/api/chat");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setTransferTimeout(300000);

    QJsonObject body;
    body["model"] = model;
    body["messages"] = messages;
    body["stream"] = true;
    if (!options.isEmpty()) body["options"] = options;

    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    m_activeReply = m_net->post(req, payload);

    connect(m_activeReply, &QNetworkReply::readyRead,
            this, &ApiClient::onChatReadyRead);
    connect(m_activeReply, &QNetworkReply::finished,
            this, &ApiClient::onChatFinished);
}

void ApiClient::onChatReadyRead()
{
    if (!m_activeReply) return;
    QByteArray data = m_activeReply->readAll();
    m_buffer.append(data);
    processBuffer();
}

void ApiClient::processBuffer()
{
    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (line.isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError) continue;

        emitChunkFromJson(doc.object());
    }
}

void ApiClient::emitChunkFromJson(const QJsonObject &obj)
{
    if (obj.contains("error")) {
        emit chatError(obj.value("error").toString());
        return;
    }

    if (obj.contains("message")) {
        QString content = obj.value("message").toObject()
                             .value("content").toString();
        if (!content.isEmpty()) {
            emit chatChunkReceived(content);
        }
    }

    if (obj.value("done").toBool() && !m_finishedEmitted) {
        m_finishedEmitted = true;
        emit chatFinished();
    }
}

void ApiClient::onChatFinished()
{
    if (!m_activeReply) return;

    QByteArray tail = m_activeReply->readAll();
    if (!tail.isEmpty()) {
        m_buffer.append(tail);
        processBuffer();
    }

    if (!m_buffer.trimmed().isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(m_buffer.trimmed(), &err);
        if (err.error == QJsonParseError::NoError) {
            emitChunkFromJson(doc.object());
        }
        m_buffer.clear();
    }

    QNetworkReply::NetworkError err = m_activeReply->error();
    QString errStr = m_activeReply->errorString();
    m_activeReply->deleteLater();
    m_activeReply = nullptr;

    if (err != QNetworkReply::NoError &&
        err != QNetworkReply::OperationCanceledError)
    {
        emit chatError(errStr);
        return;
    }

    if (!m_finishedEmitted) {
        m_finishedEmitted = true;
        emit chatFinished();
    }
}

void ApiClient::cancelChat()
{
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }
}