#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QPixmap>
#include <QHash>
#include "dlna/dlnaitem.h"

class QNetworkAccessManager;

class DlnaModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role {
        TitleRole = Qt::UserRole + 1,
        TypeRole,
        MimeTypeRole,
        ResourceUrlRole,
        ThumbnailRole,
        ItemIdRole,
        DateRole,
    };

    explicit DlnaModel(QObject *parent = nullptr);

    void setItems(const QList<DlnaItem> &items);
    void clear();

    DlnaItem itemAt(int row) const;
    const QList<DlnaItem> &items() const { return m_items; }

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void setThumbnail(int row, const QPixmap &pixmap);

private:
    QList<DlnaItem> m_items;
    QHash<int, QPixmap> m_thumbnails;
};
