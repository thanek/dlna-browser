#include <QApplication>
#include <QIcon>
#include "mainwindow.h"
#include "faicon.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FaIcon::init();
    app.setApplicationName("DlnaBrowser");
    app.setOrganizationName("dlna-browser");
    app.setApplicationDisplayName("DLNA Browser");
    app.setWindowIcon(FaIcon::icon(Fa::Server, QColor(0x5c, 0x9b, 0xd6), 64));

    MainWindow w;
    w.show();

    return app.exec();
}
