#include "faicon.h"

#include <QFontDatabase>
#include <QPainter>
#include <QPixmap>
#include <QFont>

QString FaIcon::s_family;

void FaIcon::init()
{
    int id = QFontDatabase::addApplicationFont(":/fa-solid-900.ttf");
    if (id < 0) {
        qWarning("FaIcon: failed to load fa-solid-900.ttf from resources");
        return;
    }
    QStringList families = QFontDatabase::applicationFontFamilies(id);
    if (!families.isEmpty())
        s_family = families.first();
}

QPixmap FaIcon::pixmap(uint codePoint, QColor color, int size)
{
    QPixmap px(size, size);
    px.fill(Qt::transparent);

    if (s_family.isEmpty())
        return px;

    QPainter p(&px);
    p.setRenderHint(QPainter::TextAntialiasing);

    QFont f(s_family);
    // Start at requested size and shrink until the glyph fits
    f.setPixelSize(size);
    p.setFont(f);

    QString glyph = QString::fromUcs4(&codePoint, 1);
    QFontMetrics fm(f);
    QRect br = fm.boundingRect(glyph);
    // Scale down font if glyph is wider/taller than target
    if (br.width() > size || br.height() > size) {
        double scale = qMin(double(size) / qMax(br.width(), 1),
                            double(size) / qMax(br.height(), 1));
        f.setPixelSize(qMax(1, int(size * scale)));
        p.setFont(f);
        fm = QFontMetrics(f);
        br = fm.boundingRect(glyph);
    }

    p.setPen(color);
    // Centre the glyph in the pixmap
    int x = (size - br.width()) / 2 - br.left();
    int y = (size - br.height()) / 2 - fm.ascent() - br.top() + br.height();
    p.drawText(x, y, glyph);

    return px;
}

QIcon FaIcon::icon(uint codePoint, QColor color, int /*size*/)
{
    QIcon ic;
    for (int s : {16, 20, 24, 32, 48, 64, 96, 128, 256})
        ic.addPixmap(pixmap(codePoint, color, s));
    return ic;
}
