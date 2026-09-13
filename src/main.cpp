#include "controller.h"
#include "icon.h"
#include "tray.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Ghosty"));
    app.setApplicationDisplayName(QStringLiteral("Ghosty"));
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(makeAppIcon());

    Controller controller;
    if (!controller.registerService()) {
        QMessageBox::information(
            nullptr, QStringLiteral("Ghosty"),
            QStringLiteral("Ghosty is already running in the system tray."));
        return 0;
    }

    if (!controller.ensureInstalled()) {
        QMessageBox::warning(
            nullptr, QStringLiteral("Ghosty"),
            QStringLiteral("Could not install the KWin effect assets.\n"
                           "Make sure the 'assets' folder is next to the executable."));
        return 1;
    }

    controller.apply();
    controller.rearmWatcher();

    Tray tray(&controller);

    return app.exec();
}