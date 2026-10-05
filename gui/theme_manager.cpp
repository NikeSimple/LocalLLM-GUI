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
    return QString();
}

void ThemeManager::applyCurrent()
{
    QString qss = m_dark ? loadQss(":/style_dark.qss")
                         : loadQss(":/style.qss");
    if (!qss.isEmpty()) {
        qApp->setStyleSheet(qss);
    }
}

void ThemeManager::toggle()
{
    m_dark = !m_dark;
    applyCurrent();
    emit themeChanged(m_dark);
}