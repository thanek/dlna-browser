#pragma once
#include <QMainWindow>

class DlnaModel;
class MediaViewerWidget;

class MediaViewer : public QMainWindow {
    Q_OBJECT
public:
    explicit MediaViewer(QWidget *parent = nullptr);
    void openItem(DlnaModel *model, int row);

signals:
    void rowChanged(int row);

protected:
    void closeEvent(QCloseEvent *) override;

private:
    MediaViewerWidget *m_widget;
};
