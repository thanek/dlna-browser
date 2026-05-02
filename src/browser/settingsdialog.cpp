#include "browser/settingsdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QSettings>

static constexpr auto kSeparateWindow = "mediaViewerSeparateWindow";

bool SettingsDialog::mediaViewerSeparateWindow()
{
    return QSettings().value(kSeparateWindow, false).toBool();
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    setMinimumWidth(360);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    m_chkSeparateWindow = new QCheckBox(tr("Show media viewer in separate window"), this);
    m_chkSeparateWindow->setChecked(mediaViewerSeparateWindow());

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_chkSeparateWindow);
    layout->addSpacing(8);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        QSettings().setValue(kSeparateWindow, m_chkSeparateWindow->isChecked());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
