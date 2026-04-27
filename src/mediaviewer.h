#pragma once
#include <QMainWindow>
#include "dlnaitem.h"

class QStackedWidget;
class QToolButton;
class QLabel;
class VideoWidget;
class ImageWidget;
class InfoWidget;
class DlnaModel;

class MediaViewer : public QMainWindow {
    Q_OBJECT
public:
    explicit MediaViewer(QWidget *parent = nullptr);

    // Open the item at `row` in `model`. Shows the window if hidden.
    void openItem(DlnaModel *model, int row);

protected:
    void closeEvent(QCloseEvent *) override;

private:
    void navigatePrev();
    void navigateNext();
    void updateNavButtons();
    void openRow(int row);

    QStackedWidget *m_stack;
    VideoWidget    *m_video;
    ImageWidget    *m_image;
    InfoWidget     *m_info;

    QToolButton *m_btnPrev;
    QToolButton *m_btnNext;
    QLabel      *m_infoLabel;

    DlnaModel *m_model  = nullptr;
    int        m_row    = -1;
};
