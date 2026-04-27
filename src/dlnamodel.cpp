#include "dlnamodel.h"
#include "faicon.h"
#include <QIcon>

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
        switch (item.type) {
        case DlnaItemType::Server:    return FaIcon::icon(Fa::Server,  QColor(0x5c, 0x9b, 0xd6));
        case DlnaItemType::Container: return FaIcon::icon(Fa::Folder,  QColor(0xf5, 0xa6, 0x23));
        case DlnaItemType::Video:     return FaIcon::icon(Fa::Video,   QColor(0xe5, 0x39, 0x35));
        case DlnaItemType::Audio:     return FaIcon::icon(Fa::Music,   QColor(0x8e, 0x24, 0xaa));
        case DlnaItemType::Image:     return FaIcon::icon(Fa::Image,   QColor(0x43, 0xa0, 0x47));
        default:                      return FaIcon::icon(Fa::File,    QColor(0x75, 0x75, 0x75));
        }
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
    m_thumbnails.insert(row, pixmap);
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx, {Qt::DecorationRole, ThumbnailRole});
}
