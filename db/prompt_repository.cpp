#include "prompt_repository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

PromptRepository::PromptRepository(QSqlDatabase db, QObject *parent)
    : QObject(parent), m_db(db)
{
}

int PromptRepository::create(const QString &name, const QString &text)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO prompts (name, text, is_template) "
        "VALUES (:name, :text, 1)"
    );
    query.bindValue(":name", name);
    query.bindValue(":text", text);

    if (!query.exec()) {
        qWarning() << "PromptRepository::create error:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

QVector<PromptInfo> PromptRepository::listAll()
{
    QVector<PromptInfo> result;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, text, is_template FROM prompts ORDER BY id DESC");

    if (!query.exec()) {
        qWarning() << "PromptRepository::listAll error:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        PromptInfo info;
        info.id = query.value(0).toInt();
        info.name = query.value(1).toString();
        info.text = query.value(2).toString();
        info.isTemplate = query.value(3).toInt() == 1;
        result.append(info);
    }
    return result;
}

PromptInfo PromptRepository::getById(int id)
{
    PromptInfo info;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, text, is_template FROM prompts WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) return info;

    info.id = query.value(0).toInt();
    info.name = query.value(1).toString();
    info.text = query.value(2).toString();
    info.isTemplate = query.value(3).toInt() == 1;
    return info;
}

bool PromptRepository::update(int id, const QString &name, const QString &text)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE prompts SET name = :name, text = :text WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":text", text);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "PromptRepository::update error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool PromptRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM prompts WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "PromptRepository::remove error:" << query.lastError().text();
        return false;
    }
    return true;
}