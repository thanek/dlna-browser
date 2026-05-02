#pragma once
#include <QDialog>

class QCheckBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    static bool mediaViewerSeparateWindow();

private:
    QCheckBox *m_chkSeparateWindow;
};
