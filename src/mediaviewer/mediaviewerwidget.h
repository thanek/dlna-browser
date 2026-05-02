#pragma once
#include <QWidget>
#include "mediaviewer/autohideoverlay.h"

class QStackedWidget;
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
signals:
    void closeClicked();
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
    QRectF buttonRect(int index) const;
    QRectF closeButtonRect() const;
    QRectF prevButtonRect() const;
    QRectF nextButtonRect() const;

    bool   m_prevEnabled    = false;
    bool   m_nextEnabled    = false;
    qreal  m_swipeAccumX    = 0;
    qreal  m_swipeAccumY    = 0;
    bool   m_swipeNavigated = false;

    static constexpr int NavButtonSize = 52;
};

class MediaViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit MediaViewerWidget(QWidget *parent = nullptr);
    void openItem(DlnaModel *model, int row);
    void stop();

signals:
    void closeRequested();
    void rowChanged(int row);
    void titleChanged(const QString &title);
    void infoChanged(const QString &info);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    void openRow(int row);
    void navigatePrev();
    void navigateNext();
    void updateNavButtons();

    QStackedWidget    *m_stack;
    VideoWidget       *m_video;
    ImageWidget       *m_image;
    NavButtonsOverlay *m_navOverlay;

    DlnaModel *m_model = nullptr;
    int        m_row   = -1;
};
