#pragma once
#include <QMainWindow>
#include "dlna/dlnaitem.h"
#include "mediaviewer/autohideoverlay.h"

class QStackedWidget;
class QLabel;
class QMouseEvent;
class VideoWidget;
class ImageWidget;
class DlnaModel;

class NavButtonsOverlay : public AutoHideOverlay {
    Q_OBJECT
public:
    explicit NavButtonsOverlay(QWidget *parent = nullptr);
    void setPrevEnabled(bool enabled);
    void setNextEnabled(bool enabled);
    void showButtons();

signals:
    void prevClicked();
    void nextClicked();

protected:
    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *e) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    void forwardToUnderlying(QEvent *e, QPointF globalPos);
    QRectF prevButtonRect() const;
    QRectF nextButtonRect() const;

    bool   m_prevEnabled    = false;
    bool   m_nextEnabled    = false;
    qreal  m_swipeAccumX    = 0;
    qreal  m_swipeAccumY    = 0;
    bool   m_swipeNavigated = false;

    static constexpr int NavButtonSize = 52;
};

class MediaViewer : public QMainWindow {
    Q_OBJECT
public:
    explicit MediaViewer(QWidget *parent = nullptr);
    void openItem(DlnaModel *model, int row);

signals:
    void rowChanged(int row);

protected:
    void closeEvent(QCloseEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void navigatePrev();
    void navigateNext();
    void updateNavButtons();
    void openRow(int row);

    QStackedWidget    *m_stack;
    VideoWidget       *m_video;
    ImageWidget       *m_image;
    NavButtonsOverlay *m_navOverlay;
    QLabel            *m_infoLabel;

    DlnaModel *m_model = nullptr;
    int        m_row   = -1;
};
