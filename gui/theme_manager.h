#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();

    bool isDark() const { return m_dark; }
    void toggle();
    void applyCurrent();

signals:
    void themeChanged(bool dark);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    bool m_dark = false;
    QString loadQss(const QString &path);
};

#endif