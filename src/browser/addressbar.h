#pragma once

#include <QWidget>
#include "dlna/dlnaitem.h"

class QHBoxLayout;

class AddressBar : public QWidget {
    Q_OBJECT
public:
    explicit AddressBar(QWidget *parent = nullptr);

    void setPath(const QList<DlnaLocation> &breadcrumb);

signals:
    // Emitted when user clicks a crumb. Empty list = navigate home.
    void navigateTo(QList<DlnaLocation> path);

private:
    QHBoxLayout        *m_layout;
    QList<DlnaLocation> m_breadcrumb; // full path incl. home dummy at [0]
};
