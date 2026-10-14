#ifndef MESSAGE_REPOSITORY_H
#define MESSAGE_REPOSITORY_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSqlDatabase>

/**
 * @brief Описание одного сообщения (соответствует таблице messages).
 */
struct MessageInfo {
    int id = -1;
    int dialogId = -1;
    QString role;      // "user" / "assistant" / "system"
    QString content;
    QString createdAt;
    QString attachments;
};

/**
 * @brief Репозиторий для CRUD-операций с таблицей messages.
 *
 * Соответствует разделу 4.5.1 ТЗ, реализует сохранение истории диалогов.
 */
class MessageRepository : public QObject
{
    Q_OBJECT

public:
    explicit MessageRepository(QSqlDatabase db, QObject *parent = nullptr);

    /**
     * @brief Добавляет сообщение в БД.
     * @return ID добавленного сообщения или -1 при ошибке.
     */
    int add(int dialogId, const QString &role, const QString &content);

    /**
     * @brief Возвращает все сообщения диалога, отсортированные по id ASC.
     */
    QVector<MessageInfo> listByDialog(int dialogId);

    /**
     * @brief Удаляет все сообщения диалога.
     */
    bool removeByDialog(int dialogId);

    /**
     * @brief Удаляет одно сообщение по ID.
     */
    bool remove(int id);

private:
    QSqlDatabase m_db;
};

#endif