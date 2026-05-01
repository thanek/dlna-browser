#pragma once
#include "dlna/dlnaitem.h"
#include "ui/faicon.h"
#include <QIcon>

inline QIcon dlnaIcon(DlnaItemType type)
{
    switch (type) {
    case DlnaItemType::Server:    return FaIcon::icon(Fa::Server, QColor(0x5c, 0x9b, 0xd6));
    case DlnaItemType::Container: return QIcon(":/folder.svg");
    case DlnaItemType::Video:     return FaIcon::icon(Fa::Video,  QColor(0xe5, 0x39, 0x35));
    case DlnaItemType::Audio:     return FaIcon::icon(Fa::Music,  QColor(0x8e, 0x24, 0xaa));
    case DlnaItemType::Image:     return FaIcon::icon(Fa::Image,  QColor(0x43, 0xa0, 0x47));
    default:                      return FaIcon::icon(Fa::File,   QColor(0x75, 0x75, 0x75));
    }
}
