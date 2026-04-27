#pragma once
#include <QWidget>
#include <QPixmap>
#include <QNetworkAccessManager>
#include "dlnaitem.h"

class ImageWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);
    void loadItem(const DlnaItem &item);

signals:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void setPixmap(const QPixmap &px);
    void clampOffset();
    void zoom(double factor, QPointF center = {-1, -1});
    QRectF imageRect() const;
    void drawTitleBar(QPainter &p);

    QNetworkAccessManager *m_nam;
    QPixmap m_pixmap;
    QString m_title;
    double m_zoom = 1.0;
    double m_minZoom = 1.0;
    QPointF m_offset;
    QPointF m_dragStart;
    QPointF m_offsetAtDrag;
    bool m_dragging = false;
    bool m_loading = false;

    static constexpr double ZoomStep = 1.2;
};
