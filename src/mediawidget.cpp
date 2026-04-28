#include "mediawidget.h"
#include <QPainterPath>
#include <QKeyEvent>
#include <QFontMetrics>

MediaWidget::MediaWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void MediaWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void MediaWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        emit closeRequested();
    else
        QWidget::keyPressEvent(e);
}

void MediaWidget::drawTitleBar(QPainter &p) const
{
    if (m_title.isEmpty()) return;
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = font();
    f.setPointSize(14);
    p.setFont(f);
    QFontMetrics fm(f);
    const int padH = 24, padV = 10;
    int textW = fm.horizontalAdvance(m_title);
    int boxW = qMin(textW + padH * 2, width() - 40);
    int boxH = fm.height() + padV * 2;
    QRect boxR((width() - boxW) / 2, 20, boxW, boxH);
    QPainterPath path;
    path.addRoundedRect(boxR, 20, 20);
    p.fillPath(path, QColor(0, 0, 0, 170));
    p.setPen(Qt::white);
    p.drawText(boxR, Qt::AlignCenter, m_title);
}
