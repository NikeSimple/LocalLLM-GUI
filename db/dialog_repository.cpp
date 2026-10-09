#include "dialog_repository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

DialogRepository::DialogRepository(QSqlDatabase db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

int DialogRepository::create(const QString &title, const QString &modelName)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO dialogs (title, created_at, updated_at, model_name) "
        "VALUES (:title, :created, :updated, :model)"
    );

    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    query.bindValue(":title", title);
    query.bindValue(":created", now);
    query.bindValue(":updated", now);
    query.bindValue(":model", modelName);

    if (!query.exec()) {
        qWarning() << "DialogRepository::create error:" << query.lastError().text();
        return -1;
    }

    m_lastInsertedId = query.lastInsertId().toInt();
    return m_lastInsertedId;
}

QVector<DialogInfo> DialogRepository::listAll()
{
    QVector<DialogInfo> result;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, title, created_at, updated_at, model_name, prompt_id "
        "FROM dialogs ORDER BY updated_at DESC"
    );

    if (!query.exec()) {
        qWarning() << "DialogRepository::listAll error:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        DialogInfo info;
        info.id = query.value(0).toInt();
        info.title = query.value(1).toString();
        info.createdAt = query.value(2).toString();
        info.updatedAt = query.value(3).toString();
        info.modelName = query.value(4).toString();
        info.promptId = query.value(5).isNull() ? -1 : query.value(5).toInt();
        result.append(info);
    }
    return result;
}

DialogInfo DialogRepository::getById(int id)
{
    DialogInfo info;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, title, created_at, updated_at, model_name, prompt_id "
        "FROM dialogs WHERE id = :id"
    );
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) return info;

    info.id = query.value(0).toInt();
    info.title = query.value(1).toString();
    info.createdAt = query.value(2).toString();
    info.updatedAt = query.value(3).toString();
    info.modelName = query.value(4).toString();
    info.promptId = query.value(5).isNull() ? -1 : query.value(5).toInt();
    return info;
}

bool DialogRepository::updateTitle(int id, const QString &title)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE dialogs SET title = :title, "
                  "updated_at = :updated WHERE id = :id");
    query.bindValue(":title", title);
    query.bindValue(":updated", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "DialogRepository::updateTitle error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DialogRepository::touch(int id)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE dialogs SET updated_at = :updated WHERE id = :id");
    query.bindValue(":updated", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "DialogRepository::touch error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DialogRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM dialogs WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "DialogRepository::remove error:" << query.lastError().text();
        return false;
    }
    return true;
}
