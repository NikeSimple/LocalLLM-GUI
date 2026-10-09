#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include "gui/main_window.h"
#include "gui/theme_manager.h"
#include "config/settings_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LocalLLM-GUI");
    app.setOrganizationName("NEMK");

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    app.setWindowIcon(QIcon(":/icons/app.ico"));

    int themeIdx = SettingsManager::instance()
        .value("theme", "0").toInt();
    ThemeManager::instance().setTheme(
        static_cast<ThemeManager::Theme>(themeIdx));
    ThemeManager::instance().applyCurrent();

    MainWindow window;
    window.show();

    int ret = app.exec();

    SettingsManager::instance().sync();
    return ret;
}