#ifndef DIALOG_REPOSITORY_H
#define DIALOG_REPOSITORY_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSqlDatabase>

struct DialogInfo {
    int id = -1;
    QString title;
    QString createdAt;
    QString updatedAt;
    QString modelName;
    int promptId = -1;
};

class DialogRepository : public QObject
{
    Q_OBJECT

public:
    explicit DialogRepository(QSqlDatabase db, QObject *parent = nullptr);

    int create(const QString &title, const QString &modelName);
    QVector<DialogInfo> listAll();
    DialogInfo getById(int id);
    bool updateTitle(int id, const QString &title);
    bool touch(int id);
    bool remove(int id);

    int lastInsertedId() const { return m_lastInsertedId; }

private:
    QSqlDatabase m_db;
    int m_lastInsertedId = -1;
};

#endif
