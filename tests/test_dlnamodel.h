#pragma once
#include <QObject>
#include <QTest>
#include "browser/dlnamodel.h"

class TestDlnaModel : public QObject {
    Q_OBJECT

private slots:
    void rowCount_empty();
    void rowCount_afterSetItems();
    void setItems_replacesExisting();
    void clear_emptiesModel();

    void data_displayAndTitleRole();
    void data_typeRole();
    void data_mimeTypeRole();
    void data_resourceUrlRole();
    void data_itemIdRole();
    void data_dateRole();
    void data_invalidIndex();

    void items_getter();
    void setThumbnail_outOfRange();
};
