#include "mediaviewer/mediaviewer.h"
#include "mediaviewer/videowidget.h"
#include "mediaviewer/imagewidget.h"
#include "browser/dlnamodel.h"
#include "dlna/dlnautils.h"
#include "ui/faicon.h"

#include <QStackedWidget>
#include <QToolButton>
#include <QLabel>
#include <QStatusBar>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QShortcut>

MediaViewer::MediaViewer(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_video(new VideoWidget(this))
    , m_image(new ImageWidget(this))
{
    setWindowTitle(tr("Media Viewer"));
    resize(1280, 720);
    setCentralWidget(m_stack);

    m_stack->addWidget(m_video);
    m_stack->addWidget(m_image);

    // Status bar
    m_infoLabel = new QLabel(this);
    statusBar()->addWidget(m_infoLabel, 1);

    m_btnPrev = new QToolButton(this);
    m_btnPrev->setIcon(FaIcon::icon(Fa::ArrowLeft, Qt::white));
    m_btnPrev->setToolTip(tr("Previous file (Page Up)"));
    m_btnPrev->setAutoRaise(true);

    m_btnNext = new QToolButton(this);
    m_btnNext->setIcon(FaIcon::icon(Fa::ArrowRight, Qt::white));
    m_btnNext->setToolTip(tr("Next file (Page Down)"));
    m_btnNext->setAutoRaise(true);

    statusBar()->addPermanentWidget(m_btnPrev);
    statusBar()->addPermanentWidget(m_btnNext);

    // Close on Escape from any sub-widget
    auto closeSlot = [this]() { close(); };
    connect(m_video, &VideoWidget::closeRequested, this, closeSlot);
    connect(m_image, &ImageWidget::closeRequested, this, closeSlot);

    // Keyboard navigation (PageUp/PageDown) — wired from all media widgets
    for (MediaWidget *w : {static_cast<MediaWidget*>(m_video),
                           static_cast<MediaWidget*>(m_image)}) {
        connect(w, &MediaWidget::navigatePrev, this, &MediaViewer::navigatePrev);
        connect(w, &MediaWidget::navigateNext, this, &MediaViewer::navigateNext);
    }

    connect(m_btnPrev, &QToolButton::clicked, this, &MediaViewer::navigatePrev);
    connect(m_btnNext, &QToolButton::clicked, this, &MediaViewer::navigateNext);

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
    m_infoLabel->setText(item.mimeType.isEmpty() ? QString() : item.mimeType);

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

    updateNavButtons();
}

void MediaViewer::closeEvent(QCloseEvent *e)
{
    m_video->stop();
    QMainWindow::closeEvent(e);
}

void MediaViewer::updateNavButtons()
{
    if (!m_model) { m_btnPrev->setEnabled(false); m_btnNext->setEnabled(false); return; }
    const auto &items = m_model->items();
    m_btnPrev->setEnabled(DlnaUtils::findPrevFile(items, m_row) >= 0);
    m_btnNext->setEnabled(DlnaUtils::findNextFile(items, m_row) >= 0);
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
