#pragma once
#include <QWidget>
#include <QTimer>

class AutoHideOverlay : public QWidget {
    Q_OBJECT
public:
    explicit AutoHideOverlay(QWidget *parent = nullptr, bool startVisible = false);
    void showOverlay();

protected:
    bool m_overlayVisible;

private:
    QTimer *m_hideTimer;
    static constexpr int HideDelayMs = 3000;
};
