#include "browser/dlnamodel.h"
#include "browser/dlnaicons.h"
#include <QIcon>
#include <QPainter>
#include <QApplication>
#include <QPalette>

DlnaModel::DlnaModel(QObject *parent)
    : QAbstractListModel(parent)
{}

void DlnaModel::setItems(const QList<DlnaItem> &items)
{
    beginResetModel();
    m_items = items;
    m_thumbnails.clear();
    endResetModel();
}

void DlnaModel::appendItem(const DlnaItem &item)
{
    int row = m_items.size();
    beginInsertRows({}, row, row);
    m_items.append(item);
    endInsertRows();
}

void DlnaModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_thumbnails.clear();
    endResetModel();
}

DlnaItem DlnaModel::itemAt(int row) const
{
    return m_items.value(row);
}

int DlnaModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant DlnaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const DlnaItem &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return item.title;
    case TypeRole:
        return static_cast<int>(item.type);
    case MimeTypeRole:
        return item.mimeType;
    case ResourceUrlRole:
        return item.resourceUrl;
    case ThumbnailRole:
        if (m_thumbnails.contains(index.row()))
            return m_thumbnails.value(index.row());
        return {};
    case ItemIdRole:
        return item.id;
    case DateRole:
        return item.date;
    case Qt::DecorationRole: {
        if (m_thumbnails.contains(index.row()))
            return QIcon(m_thumbnails.value(index.row()));
        return dlnaIcon(item.type);
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> DlnaModel::roleNames() const
{
    return {
        {TitleRole,       "title"},
        {TypeRole,        "type"},
        {MimeTypeRole,    "mimeType"},
        {ResourceUrlRole, "resourceUrl"},
        {ThumbnailRole,   "thumbnail"},
        {ItemIdRole,      "itemId"},
        {DateRole,        "date"},
    };
}

void DlnaModel::setThumbnail(int row, const QPixmap &pixmap)
{
    if (row < 0 || row >= m_items.size()) return;

    const int maxSize = 256;
    QPixmap px = (pixmap.width() > maxSize || pixmap.height() > maxSize)
        ? pixmap.scaled(maxSize, maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        : pixmap;

    int side = qMax(px.width(), px.height());
    QPixmap padded(side, side);
    padded.fill(Qt::transparent);
    QPainter p(&padded);
    p.drawPixmap((side - px.width()) / 2, side - px.height(), px);

    m_thumbnails.insert(row, padded);
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx, {Qt::DecorationRole, ThumbnailRole});
}
