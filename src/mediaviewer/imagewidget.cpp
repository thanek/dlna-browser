#include "mediaviewer/imagewidget.h"

#include <QPainter>
#include <QWheelEvent>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImageReader>
#include <QBuffer>

ImageWidget::ImageWidget(QWidget *parent)
    : MediaWidget(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    setMouseTracking(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
    setAutoFillBackground(true);
}

void ImageWidget::loadItem(const DlnaItem &item)
{
    setTitle(item.title);
    m_pixmap = {};
    m_zoom = 1.0;
    m_offset = {};
    m_loading = true;
    update();

    if (item.resourceUrl.isEmpty()) {
        m_loading = false;
        update();
        return;
    }

    QNetworkRequest req(item.resourceUrl);
    req.setTransferTimeout(15000);
    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_loading = false;
        if (reply->error() != QNetworkReply::NoError) { update(); return; }

        QByteArray data = reply->readAll();
        QBuffer buf(&data);
        QImageReader reader(&buf);
        reader.setAutoTransform(true);
        QImage img = reader.read();
        if (!img.isNull())
            setPixmap(QPixmap::fromImage(img));
        else
            update();
    });

    setFocus();
}

void ImageWidget::updateMinZoom()
{
    if (m_pixmap.isNull() || width() == 0 || height() == 0) return;
    double sx = double(width()) / m_pixmap.width();
    double sy = double(height()) / m_pixmap.height();
    m_minZoom = qMin(sx, sy);
}

void ImageWidget::setPixmap(const QPixmap &px)
{
    m_pixmap = px;
    updateMinZoom();
    m_zoom = m_minZoom;
    m_offset = {};
    update();
}

QRectF ImageWidget::imageRect() const
{
    if (m_pixmap.isNull()) return {};
    double w = m_pixmap.width() * m_zoom;
    double h = m_pixmap.height() * m_zoom;
    return QRectF(width() / 2.0 - w / 2.0 + m_offset.x(),
                  height() / 2.0 - h / 2.0 + m_offset.y(), w, h);
}

void ImageWidget::clampOffset()
{
    if (m_pixmap.isNull()) return;
    double imgW = m_pixmap.width() * m_zoom;
    double imgH = m_pixmap.height() * m_zoom;
    double maxX = qMax(0.0, (imgW - width()) / 2.0);
    double maxY = qMax(0.0, (imgH - height()) / 2.0);
    m_offset.setX(qBound(-maxX, m_offset.x(), maxX));
    m_offset.setY(qBound(-maxY, m_offset.y(), maxY));
}

void ImageWidget::zoom(double factor, QPointF center)
{
    if (m_pixmap.isNull()) return;
    if (center.x() < 0) center = QPointF(width() / 2.0, height() / 2.0);

    double newZoom = qMax(m_minZoom, m_zoom * factor);
    double scale = newZoom / m_zoom;
    m_offset = center + (m_offset - center) * scale;
    m_zoom = newZoom;
    clampOffset();
    update();
}

void ImageWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (!m_pixmap.isNull()) {
        updateMinZoom();
        if (m_zoom < m_minZoom) m_zoom = m_minZoom;
        clampOffset();
    }
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    if (m_loading) {
        p.setPen(QColor(0x64, 0x64, 0x64));
        QFont f = font();
        f.setPointSize(20);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "…");
        drawTitleBar(p);
        return;
    }

    if (!m_pixmap.isNull()) {
        QRectF r = imageRect();
        p.drawPixmap(r, m_pixmap, m_pixmap.rect());
    }

    drawTitleBar(p);
}

void ImageWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        if (m_zoom > m_minZoom + 0.001 || !m_offset.isNull()) {
            m_zoom = m_minZoom;
            m_offset = {};
            update();
        } else {
            emit closeRequested();
        }
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        zoom(ZoomStep);
        break;
    case Qt::Key_Minus:
        zoom(1.0 / ZoomStep);
        break;
    default:
        MediaWidget::keyPressEvent(e);
    }
}

void ImageWidget::wheelEvent(QWheelEvent *e)
{
    double factor = e->angleDelta().y() > 0 ? ZoomStep : 1.0 / ZoomStep;
    zoom(factor, e->position());
}

void ImageWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = e->position();
        m_offsetAtDrag = m_offset;
        setCursor(Qt::ClosedHandCursor);
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging) {
        m_offset = m_offsetAtDrag + (e->position() - m_dragStart);
        clampOffset();
        update();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}
