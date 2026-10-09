#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QString>
#include <QSettings>

/**
 * @brief Менеджер настроек приложения.
 *
 * Сохраняет и загружает пользовательские настройки
 * (тема, модель, имя профиля, URL сервера) в системный реестр / INI-файл.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager& instance();

    QString value(const QString &key, const QString &defaultValue = "") const;
    void setValue(const QString &key, const QString &value);

    void sync();

private:
    explicit SettingsManager(QObject *parent = nullptr);
    mutable QSettings m_settings;
};

#endif