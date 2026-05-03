#include "browser/settingsdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>

static constexpr auto kSeparateWindow = "mediaViewerSeparateWindow";
static constexpr auto kAutoplay       = "autoplay";
static constexpr auto kSeekFix        = "seekFix";

bool SettingsDialog::mediaViewerSeparateWindow()
{
    return QSettings().value(kSeparateWindow, false).toBool();
}

bool SettingsDialog::autoplay()
{
    return QSettings().value(kAutoplay, true).toBool();
}

bool SettingsDialog::seekFix()
{
    return QSettings().value(kSeekFix, true).toBool();
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    setMinimumWidth(360);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    m_chkSeparateWindow = new QCheckBox(tr("Show media viewer in separate window"), this);
    m_chkSeparateWindow->setChecked(mediaViewerSeparateWindow());

    m_chkAutoplay = new QCheckBox(tr("Autoplay media"), this);
    m_chkAutoplay->setChecked(autoplay());

    m_chkSeekFix = new QCheckBox(tr("Fix playback start with seek trick"), this);
    m_chkSeekFix->setChecked(seekFix());
    m_chkSeekFix->setEnabled(m_chkAutoplay->isChecked());

    auto *seekFixRow = new QHBoxLayout;
    seekFixRow->addSpacing(20);
    seekFixRow->addWidget(m_chkSeekFix);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_chkSeparateWindow);
    layout->addSpacing(8);
    layout->addWidget(m_chkAutoplay);
    layout->addLayout(seekFixRow);
    layout->addSpacing(8);
    layout->addWidget(buttons);

    connect(m_chkAutoplay, &QCheckBox::toggled, m_chkSeekFix, &QWidget::setEnabled);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        QSettings s;
        s.setValue(kSeparateWindow, m_chkSeparateWindow->isChecked());
        s.setValue(kAutoplay,       m_chkAutoplay->isChecked());
        s.setValue(kSeekFix,        m_chkSeekFix->isChecked());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
