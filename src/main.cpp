#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include "browser/mainwindow.h"
#include "ui/faicon.h"

static QIcon makeRoundedIcon()
{
    constexpr int sz = 512;
    QPixmap source = QIcon(":/icon.svg").pixmap(sz, sz);
    if (source.isNull())
        return {};

    // Use physical dimensions for result so DPR doesn't shrink the logical canvas.
    QPixmap result(source.width(), source.height());
    result.setDevicePixelRatio(source.devicePixelRatio());
    result.fill(Qt::transparent);

    // macOS HIG: ~10% padding each side → 80% content (logical coords, 0–512 range).
    const int margin = qRound(sz * 0.10);
    const QRectF dest(margin, margin, sz - 2 * margin, sz - 2 * margin);

    // Qt's SVG renderer ignores clip-path, so clip rounded corners manually.
    // rx ≈ 110 * 0.8 = 88 (proportional to SVG's original rx=110 at full size).
    QPainterPath clip;
    clip.addRoundedRect(dest, 88, 88);

    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipPath(clip);
    p.drawPixmap(dest.toRect(), source);

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
