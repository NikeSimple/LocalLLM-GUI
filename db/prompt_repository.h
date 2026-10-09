#ifndef PROMPT_REPOSITORY_H
#define PROMPT_REPOSITORY_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSqlDatabase>

/**
 * @brief Описание шаблона промпта (соответствует таблице prompts).
 */
struct PromptInfo {
    int id = -1;
    QString name;
    QString text;
    bool isTemplate = true;
};

/**
 * @brief Репозиторий для CRUD-операций с таблицей prompts.
 *
 * Реализует требования FR-12 ТЗ — работа с шаблонами промптов.
 */
class PromptRepository : public QObject
{
    Q_OBJECT

public:
    explicit PromptRepository(QSqlDatabase db, QObject *parent = nullptr);

    int create(const QString &name, const QString &text);
    QVector<PromptInfo> listAll();
    PromptInfo getById(int id);
    bool update(int id, const QString &name, const QString &text);
    bool remove(int id);

private:
    QSqlDatabase m_db;
};

#endif