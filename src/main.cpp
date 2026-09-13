#include "controller.h"
#include "icon.h"
#include "tray.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Opacity Slider"));
    app.setApplicationDisplayName(QStringLiteral("Opacity Slider"));
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(makeAppIcon());

    Controller controller;
    if (!controller.registerService()) {
        QMessageBox::information(
            nullptr, QStringLiteral("Opacity Slider"),
            QStringLiteral("Opacity Slider is already running in the system tray."));
        return 0;
    }

    if (!controller.ensureInstalled()) {
        QMessageBox::warning(
            nullptr, QStringLiteral("Opacity Slider"),
            QStringLiteral("Could not install the KWin effect assets.\n"
                           "Make sure the 'assets' folder is next to the executable."));
        return 1;
    }

    controller.apply();
    controller.rearmWatcher();

    Tray tray(&controller);

    return app.exec();
}