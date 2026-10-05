#include <QApplication>
#include <QStyleFactory>
#include "gui/main_window.h"
#include "gui/theme_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LocalLLM-GUI");
    app.setOrganizationName("NEMK");

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Применяем тему по умолчанию (светлая)
    ThemeManager::instance().applyCurrent();

    MainWindow window;
    window.show();

    return app.exec();
}