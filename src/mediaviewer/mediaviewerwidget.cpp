#include "mediaviewer/mediaviewerwidget.h"
#include "mediaviewer/videowidget.h"
#include "mediaviewer/imagewidget.h"
#include "browser/dlnamodel.h"
#include "dlna/dlnautils.h"
#include "ui/faicon.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QNativeGestureEvent>

// ─── NavButtonsOverlay ───────────────────────────────────────────────────────

NavButtonsOverlay::NavButtonsOverlay(QWidget *parent)
    : AutoHideOverlay(parent)
{
}

void NavButtonsOverlay::setPrevEnabled(bool enabled)
{
    if (m_prevEnabled == enabled) return;
    m_prevEnabled = enabled;
    update();
}

void NavButtonsOverlay::setNextEnabled(bool enabled)
{
    if (m_nextEnabled == enabled) return;
    m_nextEnabled = enabled;
    update();
}

QRectF NavButtonsOverlay::buttonRect(int index) const
{
    return QRectF(width() - NavButtonSize - 16, 16 + index * (NavButtonSize + 8),
                  NavButtonSize, NavButtonSize);
}

QRectF NavButtonsOverlay::closeButtonRect() const { return buttonRect(0); }
QRectF NavButtonsOverlay::prevButtonRect()  const { return buttonRect(1); }
QRectF NavButtonsOverlay::nextButtonRect()  const { return buttonRect(2); }

void NavButtonsOverlay::paintEvent(QPaintEvent *)
{
    if (!m_overlayVisible) return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    auto drawButton = [&](QRectF r, uint glyph, bool enabled) {
        if (!enabled) return;
        QPainterPath path;
        path.addEllipse(r);
        p.fillPath(path, QColor(0, 0, 0, 160));
        int iconSz = int(r.height() * 0.45);
        QPixmap px = FaIcon::pixmap(glyph, Qt::white, iconSz);
        p.drawPixmap(int(r.center().x() - px.width() / 2.0),
                     int(r.center().y() - px.height() / 2.0), px);
    };

    drawButton(closeButtonRect(), Fa::Xmark,     true);
    drawButton(prevButtonRect(),  Fa::ArrowUp,   m_prevEnabled);
    drawButton(nextButtonRect(),  Fa::ArrowDown, m_nextEnabled);
}

void NavButtonsOverlay::forwardToUnderlying(QEvent *e, QPointF globalPos)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QWidget *w = QApplication::widgetAt(globalPos.toPoint());
    if (w && w != this) {
        e->ignore();
        QApplication::sendEvent(w, e);
    }
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

bool NavButtonsOverlay::event(QEvent *e)
{
    if (e->type() == QEvent::NativeGesture) {
        auto *ge = static_cast<QNativeGestureEvent *>(e);
        if (ge->gestureType() == Qt::SwipeNativeGesture) {
            if (ge->value() > 0)
                emit prevClicked();
            else if (ge->value() < 0)
                emit nextClicked();
            return true;
        }
        forwardToUnderlying(e, ge->globalPosition());
        return true;
    }
    return QWidget::event(e);
}

void NavButtonsOverlay::wheelEvent(QWheelEvent *e)
{
    const QPoint pd = e->pixelDelta();

    // Phase handling must be outside the pixelDelta guard — ScrollEnd arrives with pd==(0,0) on macOS
    switch (e->phase()) {
    case Qt::ScrollBegin:
        m_swipeAccumX = 0;
        m_swipeAccumY = 0;
        m_swipeNavigated = false;
        break;
    case Qt::ScrollUpdate:
        if (!pd.isNull()) {
            m_swipeAccumX += pd.x();
            m_swipeAccumY += pd.y();
        }
        break;
    case Qt::ScrollEnd: {
        const qreal absX = qAbs(m_swipeAccumX);
        const qreal absY = qAbs(m_swipeAccumY);
        if (!m_swipeNavigated && absY >= 5 && absY >= absX) {
            m_swipeNavigated = true;
            if (m_swipeAccumY > 0)
                emit prevClicked();
            else
                emit nextClicked();
            return;
        }
        break;
    }
    case Qt::ScrollMomentum:
        if (m_swipeNavigated) return;
        break;
    default:
        break;
    }
    forwardToUnderlying(e, e->globalPosition());
}

void NavButtonsOverlay::mouseMoveEvent(QMouseEvent *e)
{
    showOverlay();
    forwardToUnderlying(e, e->globalPosition());
}

void NavButtonsOverlay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (closeButtonRect().contains(e->position())) {
            emit closeClicked();
            return;
        }
        if (m_prevEnabled && prevButtonRect().contains(e->position())) {
            emit prevClicked();
            return;
        }
        if (m_nextEnabled && nextButtonRect().contains(e->position())) {
            emit nextClicked();
            return;
        }
    }
    forwardToUnderlying(e, e->globalPosition());
}

void NavButtonsOverlay::mouseReleaseEvent(QMouseEvent *e)
{
    forwardToUnderlying(e, e->globalPosition());
}

// ─── MediaViewerWidget ───────────────────────────────────────────────────────

MediaViewerWidget::MediaViewerWidget(QWidget *parent)
    : QWidget(parent)
    , m_stack(new QStackedWidget(this))
    , m_video(new VideoWidget(this))
    , m_image(new ImageWidget(this))
    , m_navOverlay(new NavButtonsOverlay(m_stack))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    m_stack->addWidget(m_video);
    m_stack->addWidget(m_image);

    m_navOverlay->setGeometry(m_stack->rect());
    m_navOverlay->raise();

    connect(m_video, &VideoWidget::closeRequested, this, &MediaViewerWidget::closeRequested);
    connect(m_image, &ImageWidget::closeRequested, this, &MediaViewerWidget::closeRequested);

    for (MediaWidget *w : {static_cast<MediaWidget*>(m_video),
                           static_cast<MediaWidget*>(m_image)}) {
        connect(w, &MediaWidget::navigatePrev, this, &MediaViewerWidget::navigatePrev);
        connect(w, &MediaWidget::navigateNext, this, &MediaViewerWidget::navigateNext);
    }

    connect(m_navOverlay, &NavButtonsOverlay::closeClicked, this, &MediaViewerWidget::closeRequested);
    connect(m_navOverlay, &NavButtonsOverlay::prevClicked,  this, &MediaViewerWidget::navigatePrev);
    connect(m_navOverlay, &NavButtonsOverlay::nextClicked,  this, &MediaViewerWidget::navigateNext);

    for (auto *sc : {
        new QShortcut(QKeySequence(Qt::Key_Up),   this, [this]{ navigatePrev(); }),
        new QShortcut(QKeySequence(Qt::Key_Down), this, [this]{ navigateNext(); })
    }) sc->setContext(Qt::WidgetWithChildrenShortcut);
}

void MediaViewerWidget::stop()
{
    m_video->stop();
}

void MediaViewerWidget::openItem(DlnaModel *model, int row)
{
    m_model = model;
    openRow(row);
}

void MediaViewerWidget::openRow(int row)
{
    m_video->stop();
    m_row = row;
    emit rowChanged(row);

    DlnaItem item = m_model->itemAt(row);
    emit titleChanged(item.title.isEmpty() ? tr("Media Viewer") : item.title);

    QStringList parts;
    if (!item.resourceUrl.isEmpty()) parts << item.resourceUrl.toString();
    if (!item.mimeType.isEmpty())     parts << item.mimeType;
    if (item.fileSize > 0) {
        double mb = item.fileSize / 1048576.0;
        parts << (mb >= 1.0 ? QString::number(mb, 'f', 1) + " MB"
                            : QString::number(item.fileSize / 1024.0, 'f', 1) + " KB");
    }
    if (!item.date.isEmpty()) parts << item.date;
    emit infoChanged(parts.join("  ·  "));

    switch (item.type) {
    case DlnaItemType::Video:
    case DlnaItemType::Audio:
        m_video->loadItem(item);
        m_stack->setCurrentWidget(m_video);
        break;
    case DlnaItemType::Image:
        m_image->loadItem(item);
        m_stack->setCurrentWidget(m_image);
        break;
    default:
        return;
    }

    m_navOverlay->raise();
    updateNavButtons();
    m_navOverlay->showOverlay();
}

void MediaViewerWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_navOverlay->setGeometry(m_stack->rect());
}

void MediaViewerWidget::updateNavButtons()
{
    if (!m_model) {
        m_navOverlay->setPrevEnabled(false);
        m_navOverlay->setNextEnabled(false);
        return;
    }
    const auto &items = m_model->items();
    m_navOverlay->setPrevEnabled(DlnaUtils::findPrevFile(items, m_row) >= 0);
    m_navOverlay->setNextEnabled(DlnaUtils::findNextFile(items, m_row) >= 0);
}

void MediaViewerWidget::navigatePrev()
{
    if (!m_model) return;
    int row = DlnaUtils::findPrevFile(m_model->items(), m_row);
    if (row >= 0) openRow(row);
}

void MediaViewerWidget::navigateNext()
{
    if (!m_model) return;
    int row = DlnaUtils::findNextFile(m_model->items(), m_row);
    if (row >= 0) openRow(row);
}
