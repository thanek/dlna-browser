#pragma once

#include <QLineEdit>
#include "dlnaitem.h"

class AddressBar : public QWidget {
    Q_OBJECT
public:
    explicit AddressBar(QWidget *parent = nullptr);

    void setPath(const QList<DlnaLocation> &breadcrumb);
    QString currentPath() const;

signals:
    void pathEntered(const QString &path);

private:
    QLineEdit *m_edit;
};
