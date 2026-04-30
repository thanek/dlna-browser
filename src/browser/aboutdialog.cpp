#include "browser/aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>
#include <QApplication>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About DLNA Browser"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedWidth(360);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(28, 28, 28, 20);

    auto *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QApplication::windowIcon().pixmap(72, 72));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    layout->addSpacing(4);

    auto *nameLabel = new QLabel(QStringLiteral("<b>DLNA Browser</b>"), this);
    nameLabel->setAlignment(Qt::AlignCenter);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(nameFont.pointSize() + 4);
    nameLabel->setFont(nameFont);
    layout->addWidget(nameLabel);

    const QString version = QCoreApplication::applicationVersion();
    if (!version.isEmpty()) {
        auto *versionLabel = new QLabel(tr("Version %1").arg(version), this);
        versionLabel->setAlignment(Qt::AlignCenter);
        QPalette pal = versionLabel->palette();
        pal.setColor(QPalette::WindowText, QColor(130, 130, 130));
        versionLabel->setPalette(pal);
        layout->addWidget(versionLabel);
    }

    layout->addSpacing(8);

    auto *descLabel = new QLabel(tr("Simple multiplatform DLNA browser app"), this);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addSpacing(8);

    auto *authorLabel = new QLabel(
        tr("Author: <a href='https://github.com/thanek'>thanek</a>"), this);
    authorLabel->setAlignment(Qt::AlignCenter);
    authorLabel->setOpenExternalLinks(true);
    layout->addWidget(authorLabel);

    auto *licenseLabel = new QLabel(tr("License: GPL-3.0"), this);
    licenseLabel->setAlignment(Qt::AlignCenter);
    QPalette pal = licenseLabel->palette();
    pal.setColor(QPalette::WindowText, QColor(130, 130, 130));
    licenseLabel->setPalette(pal);
    layout->addWidget(licenseLabel);

    layout->addSpacing(16);

    auto *closeBtn = new QPushButton(tr("Close"), this);
    closeBtn->setFixedWidth(90);
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
