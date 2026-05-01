#pragma once

#include <QString>
#include <QUrl>
#include <QtGlobal>

enum class DlnaItemType {
    Server,
    Container,
    Video,
    Audio,
    Image,
    Unknown
};

struct DlnaItem {
    QString id;
    QString title;
    QString artist;
    QString album;
    DlnaItemType type = DlnaItemType::Unknown;
    QString mimeType;
    QUrl resourceUrl;
    QUrl thumbnailUrl;
    QString date;
    qint64 fileSize = 0;

    bool isContainer() const {
        return type == DlnaItemType::Server || type == DlnaItemType::Container;
    }

    static DlnaItemType typeFromMime(const QString &mime) {
        if (mime.startsWith("video/"))  return DlnaItemType::Video;
        if (mime.startsWith("audio/"))  return DlnaItemType::Audio;
        if (mime.startsWith("image/"))  return DlnaItemType::Image;
        return DlnaItemType::Unknown;
    }
};

// Represents a position in the DLNA hierarchy for navigation
struct DlnaLocation {
    QString serverName;
    QString controlUrl;   // ContentDirectory SOAP endpoint
    QString containerId;  // DLNA object ID, "0" = root
    QString title;
};
