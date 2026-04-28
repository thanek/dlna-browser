#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include "mainwindow.h"
#include "faicon.h"

static QIcon makeRoundedIcon()
{
    QPixmap source = QIcon(":/icon.svg").pixmap(512, 512);
    if (source.isNull())
        return {};

    QPixmap result(source.width(), source.height());
    result.setDevicePixelRatio(source.devicePixelRatio());
    result.fill(Qt::transparent);

    // Clip path in logical coordinates (DPR-independent: 0–512 range)
    QPainterPath clip;
    clip.addRoundedRect(QRectF(8, 8, 496, 496), 110, 110);

    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipPath(clip);
    p.drawPixmap(QPoint(0, 0), source);

    return QIcon(result);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FaIcon::init();
    app.setApplicationName("DlnaBrowser");
    app.setOrganizationName("dlna-browser");
    app.setApplicationDisplayName("DLNA Browser");
    app.setWindowIcon(makeRoundedIcon());

    MainWindow w;
    w.show();

    return app.exec();
}
