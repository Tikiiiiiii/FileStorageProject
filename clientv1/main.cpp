#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // 对于使用 Wayland 作为桌面显示的情况，确实有些应用程序可能会遇到无法拖动或定位无边框窗体的问题，
    // 特别是在一些桌面环境和窗口管理器下。
    // Wayland 不像 X11 那样有传统的坐标系统，这可能导致一些 Qt 程序 如 move 函数 默认在 Wayland 下的行为不如预期。
#ifdef _WIN32
#else
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "client_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();

    return a.exec();
}
