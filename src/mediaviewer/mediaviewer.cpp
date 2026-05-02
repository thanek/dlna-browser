#include "mediaviewer/mediaviewer.h"
#include "mediaviewer/mediaviewerwidget.h"

#include <QLabel>
#include <QStatusBar>
#include <QCloseEvent>
#include <QEvent>

MediaViewer::MediaViewer(QWidget *parent)
    : QMainWindow(parent)
    , m_widget(new MediaViewerWidget(this))
{
    setWindowTitle(tr("Media Viewer"));
    resize(1280, 720);
    setCentralWidget(m_widget);

    auto *infoLabel = new QLabel(this);
    statusBar()->addWidget(infoLabel, 1);

    connect(m_widget, &MediaViewerWidget::closeRequested,          this, &QWidget::close);
    connect(m_widget, &MediaViewerWidget::fullscreenToggleRequested, this, [this] {
        isFullScreen() ? showNormal() : showFullScreen();
    });
    connect(m_widget, &MediaViewerWidget::rowChanged,     this, &MediaViewer::rowChanged);
    connect(m_widget, &MediaViewerWidget::titleChanged,   this, &QWidget::setWindowTitle);
    connect(m_widget, &MediaViewerWidget::infoChanged,    infoLabel, &QLabel::setText);
}

void MediaViewer::openItem(DlnaModel *model, int row)
{
    m_widget->openItem(model, row);
    show();
    raise();
    activateWindow();
}

void MediaViewer::closeEvent(QCloseEvent *e)
{
    m_widget->stop();
    QMainWindow::closeEvent(e);
}

void MediaViewer::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    if (e->type() == QEvent::WindowStateChange) {
        const bool fs = isFullScreen();
        m_widget->setFullscreen(fs);
        statusBar()->setVisible(!fs);
    }
}
