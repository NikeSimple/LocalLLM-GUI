#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);

    void setBaseUrl(const QString &url) { m_baseUrl = url; }
    QString baseUrl() const { return m_baseUrl; }

    void ping();
    void listModels();
    void sendChat(const QString &model,
                  const QJsonArray &messages,
                  const QJsonObject &options);
    void generateTitle(const QString &model, const QString &userText);
    void cancelChat();

signals:
    void pongReceived(bool ok, int modelCount);
    void modelsReceived(const QStringList &models);
    void chatChunkReceived(const QString &chunk);
    void chatFinished();
    void chatError(const QString &error);
    void titleGenerated(const QString &title);

private slots:
    void onChatReadyRead();
    void onChatFinished();

private:
    void processBuffer();
    void emitChunkFromJson(const QJsonObject &obj);

    QNetworkAccessManager *m_net = nullptr;
    QString m_baseUrl = "http://localhost:11434";

    QNetworkReply *m_activeReply = nullptr;
    QByteArray m_buffer;
    bool m_finishedEmitted = false;
};

#endif