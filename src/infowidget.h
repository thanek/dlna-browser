#pragma once
#include <QWidget>
#include "dlnaitem.h"

class InfoWidget : public QWidget {
    Q_OBJECT
public:
    explicit InfoWidget(QWidget *parent = nullptr);
    void showItem(const DlnaItem &item);

signals:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    DlnaItem m_item;
};
