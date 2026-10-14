#include "message_repository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

MessageRepository::MessageRepository(QSqlDatabase db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

int MessageRepository::add(int dialogId, const QString &role, const QString &content)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO messages (dialog_id, role, content, created_at) "
        "VALUES (:dialog_id, :role, :content, :created_at)"
    );
    query.bindValue(":dialog_id", dialogId);
    query.bindValue(":role", role);
    query.bindValue(":content", content);
    query.bindValue(":created_at",
                    QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning() << "MessageRepository::add error:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

QVector<MessageInfo> MessageRepository::listByDialog(int dialogId)
{
    QVector<MessageInfo> result;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, dialog_id, role, content, created_at, attachments "
        "FROM messages WHERE dialog_id = :dialog_id ORDER BY id ASC"
    );
    query.bindValue(":dialog_id", dialogId);

    if (!query.exec()) {
        qWarning() << "MessageRepository::listByDialog error:"
                   << query.lastError().text();
        return result;
    }

    while (query.next()) {
        MessageInfo info;
        info.id = query.value(0).toInt();
        info.dialogId = query.value(1).toInt();
        info.role = query.value(2).toString();
        info.content = query.value(3).toString();
        info.createdAt = query.value(4).toString();
        info.attachments = query.value(5).toString();
        result.append(info);
    }
    return result;
}

bool MessageRepository::removeByDialog(int dialogId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE dialog_id = :dialog_id");
    query.bindValue(":dialog_id", dialogId);
    if (!query.exec()) {
        qWarning() << "MessageRepository::removeByDialog error:"
                   << query.lastError().text();
        return false;
    }
    return true;
}

bool MessageRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "MessageRepository::remove error:"
                   << query.lastError().text();
        return false;
    }
    return true;
}