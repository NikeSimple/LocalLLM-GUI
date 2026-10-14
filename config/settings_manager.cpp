#include "settings_manager.h"

SettingsManager& SettingsManager::instance()
{
    static SettingsManager mgr;
    return mgr;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
      m_settings("NEMK", "LocalLLM-GUI")
{
}

QString SettingsManager::value(const QString &key, const QString &defaultValue) const
{
    return m_settings.value(key, defaultValue).toString();
}

void SettingsManager::setValue(const QString &key, const QString &value)
{
    m_settings.setValue(key, value);
}

void SettingsManager::sync()
{
    m_settings.sync();
}