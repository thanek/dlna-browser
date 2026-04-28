#pragma once
#include "dlnaitem.h"
#include "mediawidget.h"

class InfoWidget : public MediaWidget {
    Q_OBJECT
public:
    explicit InfoWidget(QWidget *parent = nullptr);
    void showItem(const DlnaItem &item);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    DlnaItem m_item;
};
