#pragma once
#include <QPixmap>
#include <QNetworkAccessManager>
#include "dlna/dlnaitem.h"
#include "mediaviewer/mediawidget.h"

class ImageWidget : public MediaWidget {
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);
    void loadItem(const DlnaItem &item);

protected:
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void setPixmap(const QPixmap &px);
    void updateMinZoom();
    void clampOffset();
    void zoom(double factor, QPointF center = {-1, -1});
    QRectF imageRect() const;

    QNetworkAccessManager *m_nam;
    QPixmap m_pixmap;
    double m_zoom = 1.0;
    double m_minZoom = 1.0;
    QPointF m_offset;
    QPointF m_dragStart;
    QPointF m_offsetAtDrag;
    bool m_dragging = false;
    bool m_loading = false;

    static constexpr double ZoomStep = 1.2;
};
