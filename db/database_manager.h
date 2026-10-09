#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool open(const QString &path = "localllm.db");
    void close();
    QSqlDatabase database() const { return m_db; }
    bool createTables();

private:
    QSqlDatabase m_db;
};

#endif
