#include "mediaviewer/mediaviewer.h"
#include "mediaviewer/videowidget.h"
#include "mediaviewer/imagewidget.h"
#include "browser/dlnamodel.h"
#include "dlna/dlnautils.h"
#include "ui/faicon.h"

#include <QStackedWidget>
#include <QLabel>
#include <QStatusBar>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QShortcut>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QMouseEvent>

// ─── NavButtonsOverlay ───────────────────────────────────────────────────────

NavButtonsOverlay::NavButtonsOverlay(QWidget *parent)
    : QWidget(parent)
    , m_hideTimer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(3000);
    connect(m_hideTimer, &QTimer::timeout, this, [this]() {
        m_visible = false;
        update();
    });
}

void NavButtonsOverlay::setPrevEnabled(bool enabled)
{
    m_prevEnabled = enabled;
    update();
}

void NavButtonsOverlay::setNextEnabled(bool enabled)
{
    m_nextEnabled = enabled;
    update();
}

void NavButtonsOverlay::showButtons()
{
    m_visible = true;
    m_hideTimer->start();
    update();
}

QRectF NavButtonsOverlay::prevButtonRect() const
{
    const int sz = 52;
    return QRectF(16, (height() - sz) / 2.0, sz, sz);
}

QRectF NavButtonsOverlay::nextButtonRect() const
{
    const int sz = 52;
    return QRectF(width() - sz - 16, (height() - sz) / 2.0, sz, sz);
}

void NavButtonsOverlay::paintEvent(QPaintEvent *)
{
    if (!m_visible) return;
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

    drawButton(prevButtonRect(), Fa::ArrowLeft,  m_prevEnabled);
    drawButton(nextButtonRect(), Fa::ArrowRight, m_nextEnabled);
}

void NavButtonsOverlay::forwardToUnderlying(QMouseEvent *e)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QWidget *w = QApplication::widgetAt(e->globalPosition().toPoint());
    if (w && w != this) {
        QMouseEvent ev(e->type(),
                       w->mapFromGlobal(e->globalPosition()),
                       e->globalPosition(),
                       e->button(), e->buttons(), e->modifiers());
        QApplication::sendEvent(w, &ev);
    }
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void NavButtonsOverlay::mouseMoveEvent(QMouseEvent *e)
{
    showButtons();
    forwardToUnderlying(e);
}

void NavButtonsOverlay::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (m_prevEnabled && prevButtonRect().contains(e->position())) {
            emit prevClicked();
            return;
        }
        if (m_nextEnabled && nextButtonRect().contains(e->position())) {
            emit nextClicked();
            return;
        }
    }
    forwardToUnderlying(e);
}

void NavButtonsOverlay::mouseReleaseEvent(QMouseEvent *e)
{
    forwardToUnderlying(e);
}

// ─── MediaViewer ─────────────────────────────────────────────────────────────

MediaViewer::MediaViewer(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_video(new VideoWidget(this))
    , m_image(new ImageWidget(this))
    , m_navOverlay(new NavButtonsOverlay(m_stack))
{
    setWindowTitle(tr("Media Viewer"));
    resize(1280, 720);
    setCentralWidget(m_stack);

    m_stack->addWidget(m_video);
    m_stack->addWidget(m_image);

    m_navOverlay->setGeometry(m_stack->rect());
    m_navOverlay->raise();

    // Status bar — file info only
    m_infoLabel = new QLabel(this);
    statusBar()->addWidget(m_infoLabel, 1);

    // Close on Escape from any sub-widget
    auto closeSlot = [this]() { close(); };
    connect(m_video, &VideoWidget::closeRequested, this, closeSlot);
    connect(m_image, &ImageWidget::closeRequested, this, closeSlot);

    // Keyboard navigation (PageUp/PageDown)
    for (MediaWidget *w : {static_cast<MediaWidget*>(m_video),
                           static_cast<MediaWidget*>(m_image)}) {
        connect(w, &MediaWidget::navigatePrev, this, &MediaViewer::navigatePrev);
        connect(w, &MediaWidget::navigateNext, this, &MediaViewer::navigateNext);
    }

    connect(m_navOverlay, &NavButtonsOverlay::prevClicked, this, &MediaViewer::navigatePrev);
    connect(m_navOverlay, &NavButtonsOverlay::nextClicked, this, &MediaViewer::navigateNext);

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left),  this, [this]{ navigatePrev(); });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right), this, [this]{ navigateNext(); });
}

void MediaViewer::openItem(DlnaModel *model, int row)
{
    m_model = model;
    openRow(row);
    show();
    raise();
    activateWindow();
}

void MediaViewer::openRow(int row)
{
    m_video->stop();
    m_row = row;
    emit rowChanged(row);

    DlnaItem item = m_model->itemAt(row);
    setWindowTitle(item.title.isEmpty() ? tr("Media Viewer") : item.title);

    QString displayTitle = item.title;
    if (item.type == DlnaItemType::Audio && !item.artist.isEmpty())
        displayTitle = item.artist + " — " + item.title;

    QStringList parts;
    if (!item.resourceUrl.isEmpty()) parts << item.resourceUrl.toString();
    if (!item.mimeType.isEmpty()) parts << item.mimeType;
    if (item.fileSize > 0) {
        double mb = item.fileSize / 1048576.0;
        parts << (mb >= 1.0 ? QString::number(mb, 'f', 1) + " MB"
                            : QString::number(item.fileSize / 1024.0, 'f', 1) + " KB");
    }
    if (!item.date.isEmpty()) parts << item.date;
    m_infoLabel->setText(parts.join("  ·  "));

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
}

void MediaViewer::closeEvent(QCloseEvent *e)
{
    m_video->stop();
    QMainWindow::closeEvent(e);
}

void MediaViewer::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    m_navOverlay->setGeometry(m_stack->rect());
}

void MediaViewer::updateNavButtons()
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

void MediaViewer::navigatePrev()
{
    if (!m_model) return;
    int row = DlnaUtils::findPrevFile(m_model->items(), m_row);
    if (row >= 0) openRow(row);
}

void MediaViewer::navigateNext()
{
    if (!m_model) return;
    int row = DlnaUtils::findNextFile(m_model->items(), m_row);
    if (row >= 0) openRow(row);
}
