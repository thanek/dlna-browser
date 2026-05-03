#pragma once
#include <QDialog>

class QCheckBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    static bool mediaViewerSeparateWindow();
    static bool autoplay();
    static bool seekFix();

private:
    QCheckBox *m_chkSeparateWindow;
    QCheckBox *m_chkAutoplay;
    QCheckBox *m_chkSeekFix;
};
