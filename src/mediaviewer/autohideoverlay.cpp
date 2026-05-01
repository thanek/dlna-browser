#include "mediaviewer/autohideoverlay.h"

AutoHideOverlay::AutoHideOverlay(QWidget *parent, bool startVisible)
    : QWidget(parent)
    , m_overlayVisible(startVisible)
    , m_hideTimer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(HideDelayMs);
    connect(m_hideTimer, &QTimer::timeout, this, [this]() {
        m_overlayVisible = false;
        update();
    });
}

void AutoHideOverlay::showOverlay()
{
    m_overlayVisible = true;
    m_hideTimer->start();
    update();
}
