#include "theme_manager.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent) {}

QString ThemeManager::loadQss(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QString content = QString::fromUtf8(f.readAll());
        f.close();
        return content;
    }
    qWarning() << "Не удалось загрузить QSS:" << path;
    return QString();
}

QString ThemeManager::qssPath(Theme t) const
{
    switch (t) {
        case Light: return ":/style.qss";
        case Dark:  return ":/style_dark.qss";
        case Red:   return ":/style_red.qss";
        case Ocean: return ":/style_ocean.qss";
    }
    return ":/style.qss";
}

void ThemeManager::applyCurrent()
{
    QString qss = loadQss(qssPath(m_theme));
    if (!qss.isEmpty()) {
        qApp->setStyleSheet(qss);
    }
}

void ThemeManager::setTheme(Theme t)
{
    if (m_theme == t) return;
    m_theme = t;
    applyCurrent();
    emit themeChanged();
}

void ThemeManager::toggle()
{
    setTheme(m_theme == Dark ? Light : Dark);
}

bool ThemeManager::isDark() const
{
    return m_theme == Dark || m_theme == Red || m_theme == Ocean;
}

QString ThemeManager::themeDisplayName(Theme t)
{
    switch (t) {
        case Light: return "Светлая";
        case Dark:  return "Тёмная";
        case Red:   return "Красная";
        case Ocean: return "Океан";
    }
    return "Светлая";
}