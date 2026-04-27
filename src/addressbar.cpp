#include "addressbar.h"

AddressBar::AddressBar(QWidget *parent)
    : QWidget(parent)
    , m_edit(new QLineEdit(this))
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_edit);

    m_edit->setPlaceholderText(tr("DLNA path…"));
    m_edit->setReadOnly(true);

    connect(m_edit, &QLineEdit::returnPressed, this, [this]() {
        emit pathEntered(m_edit->text());
    });
}

void AddressBar::setPath(const QList<DlnaLocation> &breadcrumb)
{
    QStringList parts;
    for (const auto &loc : breadcrumb)
        parts.append(loc.title.isEmpty() ? loc.serverName : loc.title);
    m_edit->setText(parts.join(" / "));
}

QString AddressBar::currentPath() const
{
    return m_edit->text();
}
