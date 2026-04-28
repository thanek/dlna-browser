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
    app.setWindowIcon(QIcon(":/icon.svg"));

    MainWindow w;
    w.show();

    return app.exec();
}
