#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        Light = 0,
        Dark = 1,
        Red = 2,
        Ocean = 3
    };

    static ThemeManager& instance();

    Theme currentTheme() const { return m_theme; }
    void setTheme(Theme t);
    void toggle();
    void applyCurrent();
    bool isDark() const;

    static QString themeDisplayName(Theme t);

signals:
    void themeChanged();

private:
    explicit ThemeManager(QObject *parent = nullptr);
    Theme m_theme = Light;
    QString loadQss(const QString &path);
    QString qssPath(Theme t) const;
};

#endif