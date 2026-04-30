#include "ui/faicon.h"

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

QPixmap FaIcon::pixmap(uint codePoint, QColor color, int size, double scale)
{
    QPixmap px(size, size);
    px.fill(Qt::transparent);

    if (s_family.isEmpty())
        return px;

    QPainter p(&px);
    p.setRenderHint(QPainter::TextAntialiasing);

    const int maxGlyph = qMax(1, int(size * scale));

    QFont f(s_family);
    f.setPixelSize(maxGlyph);
    p.setFont(f);

    QString glyph = QString::fromUcs4(reinterpret_cast<const char32_t *>(&codePoint), 1);
    QFontMetrics fm(f);
    QRect br = fm.boundingRect(glyph);
    if (br.width() > maxGlyph || br.height() > maxGlyph) {
        double fit = qMin(double(maxGlyph) / qMax(br.width(), 1),
                          double(maxGlyph) / qMax(br.height(), 1));
        f.setPixelSize(qMax(1, int(maxGlyph * fit)));
        p.setFont(f);
        fm = QFontMetrics(f);
        br = fm.boundingRect(glyph);
    }

    p.setPen(color);
    int x = (size - br.width()) / 2 - br.left();
    int y = (size - br.height()) / 2 - fm.ascent() - br.top() + br.height();
    p.drawText(x, y, glyph);

    return px;
}

QIcon FaIcon::icon(uint codePoint, QColor color, int /*size*/, double scale)
{
    QIcon ic;
    for (int s : {16, 20, 24, 32, 48, 64, 96, 128, 256})
        ic.addPixmap(pixmap(codePoint, color, s, scale));
    return ic;
}
