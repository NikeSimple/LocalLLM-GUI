#include "database_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qWarning() << "Не удалось открыть БД:" << m_db.lastError().text();
        return false;
    }

    // Раздел 4.5.5 ТЗ — целостность данных
    QSqlQuery pragma(m_db);
    if (!pragma.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Не удалось включить foreign_keys:" << pragma.lastError().text();
    }

    qDebug() << "БД открыта:" << path;
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::createTables()
{
    // QSQLITE не принимает несколько SQL-запросов в одном exec().
    // Поэтому каждый CREATE выполняем отдельно.

    QStringList statements;

    // Таблица dialogs (раздел 4.5.1 ТЗ)
    statements << QString(
        "CREATE TABLE IF NOT EXISTS dialogs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "model_name TEXT,"
        "prompt_id INTEGER,"
        "FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE SET NULL"
        ")"
    );

    // Таблица messages
    statements << QString(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "dialog_id INTEGER NOT NULL,"
        "role TEXT NOT NULL,"
        "content TEXT NOT NULL,"
        "created_at TEXT NOT NULL,"
        "attachments TEXT,"
        "FOREIGN KEY (dialog_id) REFERENCES dialogs(id) ON DELETE CASCADE"
        ")"
    );

    // Таблица prompts
    statements << QString(
        "CREATE TABLE IF NOT EXISTS prompts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "text TEXT NOT NULL,"
        "is_template INTEGER DEFAULT 0"
        ")"
    );

    // Таблица settings
    statements << QString(
        "CREATE TABLE IF NOT EXISTS settings ("
        "key TEXT PRIMARY KEY,"
        "value TEXT"
        ")"
    );

    // Индексы (раздел 4.5.3 ТЗ)
    statements << QString(
        "CREATE INDEX IF NOT EXISTS idx_messages_dialog_id "
        "ON messages(dialog_id)"
    );

    statements << QString(
        "CREATE INDEX IF NOT EXISTS idx_dialogs_updated_at "
        "ON dialogs(updated_at)"
    );

    statements << QString(
        "CREATE INDEX IF NOT EXISTS idx_dialogs_prompt_id "
        "ON dialogs(prompt_id)"
    );

    QSqlQuery query(m_db);

    for (const QString &sql : statements) {
        if (!query.exec(sql)) {
            qWarning() << "Ошибка SQL:" << query.lastError().text();
            qWarning() << "Запрос:" << sql;
            return false;
        }
    }

    qDebug() << "Все таблицы созданы/проверены";
    return true;
}