#include "browser/addressbar.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>

AddressBar::AddressBar(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(2, 0, 2, 0);
    m_layout->setSpacing(1);
    m_layout->addStretch();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void AddressBar::setPath(const QList<DlnaLocation> &breadcrumb)
{
    m_breadcrumb = breadcrumb;

    QLayoutItem *item;
    while ((item = m_layout->takeAt(0))) {
        delete item->widget();
        delete item;
    }

    for (int i = 0; i < breadcrumb.size(); ++i) {
        if (i > 0) {
            auto *sep = new QLabel("›", this);
            sep->setContentsMargins(2, 0, 2, 0);
            m_layout->addWidget(sep);
        }

        const QString text = breadcrumb[i].title.isEmpty()
                             ? breadcrumb[i].serverName
                             : breadcrumb[i].title;

        const bool isLast = (i == breadcrumb.size() - 1);

        if (isLast) {
            auto *lbl = new QLabel(text, this);
            QFont f = lbl->font();
            f.setBold(true);
            lbl->setFont(f);
            m_layout->addWidget(lbl);
        } else {
            auto *btn = new QToolButton(this);
            btn->setText(text);
            btn->setAutoRaise(true);
            connect(btn, &QToolButton::clicked, this, [this, i]() {
                if (i == 0) {
                    emit navigateTo({});
                } else {
                    // breadcrumb[0] is home dummy, real path starts at [1]
                    emit navigateTo(m_breadcrumb.mid(1, i));
                }
            });
            m_layout->addWidget(btn);
        }
    }

    m_layout->addStretch();
}
