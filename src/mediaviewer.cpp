#include "mediaviewer.h"
#include "videowidget.h"
#include "imagewidget.h"
#include "infowidget.h"
#include "dlnamodel.h"
#include "faicon.h"

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
    , m_info(new InfoWidget(this))
{
    setWindowTitle(tr("Media Viewer"));
    resize(1280, 720);
    setCentralWidget(m_stack);

    m_stack->addWidget(m_video);
    m_stack->addWidget(m_image);
    m_stack->addWidget(m_info);

    // Status bar
    m_infoLabel = new QLabel(this);
    statusBar()->addWidget(m_infoLabel, 1);

    m_btnPrev = new QToolButton(this);
    m_btnPrev->setIcon(FaIcon::icon(Fa::ArrowLeft, Qt::white, 16));
    m_btnPrev->setToolTip(tr("Previous file (Page Up)"));
    m_btnPrev->setAutoRaise(true);

    m_btnNext = new QToolButton(this);
    m_btnNext->setIcon(FaIcon::icon(Fa::ArrowRight, Qt::white, 16));
    m_btnNext->setToolTip(tr("Next file (Page Down)"));
    m_btnNext->setAutoRaise(true);

    statusBar()->addPermanentWidget(m_btnPrev);
    statusBar()->addPermanentWidget(m_btnNext);

    // Close on Escape from any sub-widget
    auto closeSlot = [this]() { close(); };
    connect(m_video, &VideoWidget::closeRequested, this, closeSlot);
    connect(m_image, &ImageWidget::closeRequested, this, closeSlot);
    connect(m_info,  &InfoWidget::closeRequested,  this, closeSlot);

    // Keyboard navigation (PageUp/PageDown) — wired from all media widgets
    for (MediaWidget *w : {static_cast<MediaWidget*>(m_video),
                           static_cast<MediaWidget*>(m_image),
                           static_cast<MediaWidget*>(m_info)}) {
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
        m_info->showItem(item);
        m_stack->setCurrentWidget(m_info);
        break;
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

    int prev = m_row - 1;
    while (prev >= 0 && m_model->itemAt(prev).isContainer()) --prev;
    m_btnPrev->setEnabled(prev >= 0);

    int next = m_row + 1;
    while (next < m_model->rowCount() && m_model->itemAt(next).isContainer()) ++next;
    m_btnNext->setEnabled(next < m_model->rowCount());
}

void MediaViewer::navigatePrev()
{
    if (!m_model) return;
    int row = m_row - 1;
    while (row >= 0 && m_model->itemAt(row).isContainer()) --row;
    if (row >= 0) openRow(row);
}

void MediaViewer::navigateNext()
{
    if (!m_model) return;
    int row = m_row + 1;
    while (row < m_model->rowCount() && m_model->itemAt(row).isContainer()) ++row;
    if (row < m_model->rowCount()) openRow(row);
}
