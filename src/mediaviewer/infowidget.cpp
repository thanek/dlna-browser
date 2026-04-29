#include "mediaviewer/infowidget.h"
#include "browser/dlnaicons.h"

#include <QPainter>
#include <QPainterPath>
#include <QIcon>
#include <QFontMetrics>

InfoWidget::InfoWidget(QWidget *parent) : MediaWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
}

void InfoWidget::showItem(const DlnaItem &item)
{
    m_item = item;
    update();
    setFocus();
}

void InfoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    p.fillRect(rect(), QColor(0, 0, 0));

    const int cardW = qMin(480, width() - 64);
    const int cx = width() / 2;
    const int iconSz = 72;

    QRect iconRect(cx - iconSz / 2, height() / 2 - 160, iconSz, iconSz);
    dlnaIcon(m_item.type, 72).paint(&p, iconRect);

    QFont titleFont = font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.setPen(Qt::white);
    QRect titleRect(cx - cardW / 2, iconRect.bottom() + 20, cardW, 80);
    p.drawText(titleRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
               m_item.title.isEmpty() ? tr("Unknown") : m_item.title);

    const int cardY = titleRect.top() + 80;
    const int rowH = 32;
    int cardH = rowH * 2 + 32;
    if (!m_item.mimeType.isEmpty()) cardH += rowH;
    if (!m_item.resourceUrl.isEmpty()) cardH += rowH;

    QRect cardRect(cx - cardW / 2, cardY, cardW, cardH);
    QPainterPath path;
    path.addRoundedRect(cardRect, 10, 10);
    p.fillPath(path, QColor(22, 22, 22));

    QFont labelFont = font();
    labelFont.setPointSize(12);
    QFont valueFont = font();
    valueFont.setPointSize(12);

    auto drawRow = [&](int y, const QString &label, const QString &value) {
        p.setFont(labelFont);
        p.setPen(QColor(0x55, 0x55, 0x55));
        p.drawText(cardRect.left() + 20, y, cardW / 2 - 20, rowH,
                   Qt::AlignVCenter | Qt::AlignLeft, label);
        p.setFont(valueFont);
        p.setPen(QColor(0xaa, 0xaa, 0xaa));
        p.drawText(cardRect.left() + cardW / 2, y, cardW / 2 - 20, rowH,
                   Qt::AlignVCenter | Qt::AlignLeft, value);
    };

    int rowY = cardRect.top() + 16;

    QString typeStr = m_item.mimeType.isEmpty() ? tr("unknown") : m_item.mimeType;
    drawRow(rowY, tr("Type"), typeStr);
    rowY += rowH;

    if (!m_item.resourceUrl.isEmpty()) {
        QString urlStr = m_item.resourceUrl.toString();
        if (urlStr.length() > 48) urlStr = "…" + urlStr.right(46);
        drawRow(rowY, tr("URL"), urlStr);
        rowY += rowH;
    }

    if (!m_item.date.isEmpty()) {
        drawRow(rowY, tr("Date"), m_item.date);
    }

    QFont hintFont = font();
    hintFont.setPointSize(11);
    hintFont.setItalic(true);
    p.setFont(hintFont);
    p.setPen(QColor(0x44, 0x44, 0x44));
    p.drawText(0, cardRect.bottom() + 16, width(), 24,
               Qt::AlignHCenter, tr("Press Escape to go back"));
}
