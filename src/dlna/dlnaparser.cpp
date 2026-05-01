#include "dlna/dlnaparser.h"
#include <QDomDocument>
#include <QUrl>

namespace DlnaParser {

// ─── DIDL-Lite ───────────────────────────────────────────────────────────────

static void parseCommonFields(QDomElement &el, DlnaItem &item)
{
    item.id = el.attribute("id");

    auto titles = el.elementsByTagName("dc:title");
    if (!titles.isEmpty())
        item.title = titles.at(0).toElement().text();
    if (item.title.isEmpty()) {
        auto plain = el.elementsByTagName("title");
        if (!plain.isEmpty()) item.title = plain.at(0).toElement().text();
    }

    auto arts = el.elementsByTagName("upnp:albumArtURI");
    if (!arts.isEmpty())
        item.thumbnailUrl = QUrl(arts.at(0).toElement().text());

    auto dates = el.elementsByTagName("dc:date");
    if (!dates.isEmpty())
        item.date = dates.at(0).toElement().text();
}

QList<DlnaItem> parseDidl(const QString &didlXml)
{
    QList<DlnaItem> items;

    QDomDocument didl;
    if (!didl.setContent(didlXml))
        return items;

    auto containers = didl.elementsByTagName("container");
    for (int i = 0; i < containers.count(); ++i) {
        QDomElement el = containers.at(i).toElement();
        DlnaItem item;
        item.type = DlnaItemType::Container;
        parseCommonFields(el, item);
        items.append(item);
    }

    auto entries = didl.elementsByTagName("item");
    for (int i = 0; i < entries.count(); ++i) {
        QDomElement el = entries.at(i).toElement();
        DlnaItem item;
        parseCommonFields(el, item);

        auto resources = el.elementsByTagName("res");
        for (int r = 0; r < resources.count(); ++r) {
            QDomElement res = resources.at(r).toElement();
            QStringList parts = res.attribute("protocolInfo").split(':');
            QString mime  = parts.value(2);
            QString extra = parts.value(3);

            bool isThumbnail = extra.contains("JPEG_TN") || extra.contains("PNG_TN");
            if (isThumbnail) {
                if (item.thumbnailUrl.isEmpty())
                    item.thumbnailUrl = QUrl(res.text());
            } else if (item.resourceUrl.isEmpty()) {
                item.resourceUrl = QUrl(res.text());
                item.mimeType    = mime;
                item.type        = DlnaItem::typeFromMime(mime);
                QString sizeStr  = res.attribute("size");
                if (!sizeStr.isEmpty())
                    item.fileSize = sizeStr.toLongLong();
            }
        }
        items.append(item);
    }

    return items;
}

QList<DlnaItem> parseBrowseResponse(const QByteArray &soapResponse)
{
    QDomDocument envelope;
    if (!envelope.setContent(soapResponse))
        return {};

    auto results = envelope.elementsByTagName("Result");
    if (results.isEmpty())
        return {};

    return parseDidl(results.at(0).toElement().text());
}

// ─── Device descriptor ───────────────────────────────────────────────────────

DlnaServer parseDeviceDescriptor(const QByteArray &descriptorXml,
                                  const QString &location,
                                  const QString &usn)
{
    DlnaServer server;
    QDomDocument doc;
    if (!doc.setContent(descriptorXml))
        return server;

    auto findText = [&](const QDomElement &root, const QString &tag) -> QString {
        auto list = root.elementsByTagName(tag);
        return list.isEmpty() ? QString{} : list.at(0).toElement().text();
    };

    QDomElement root = doc.documentElement();
    server.name = findText(root, "friendlyName");
    if (server.name.isEmpty())
        return server;

    server.location = location;
    server.usn      = usn;

    QString baseStr = QUrl(location).toString(QUrl::RemovePath | QUrl::RemoveQuery);
    auto iconLists = root.elementsByTagName("iconList");
    if (!iconLists.isEmpty())
        server.iconUrl = bestIconUrl(iconLists.at(0).toElement(), baseStr);

    return server;
}

QString parseControlUrl(const QByteArray &descriptorXml, const QString &baseUrl)
{
    QDomDocument doc;
    if (!doc.setContent(descriptorXml))
        return {};

    auto services = doc.elementsByTagName("service");
    for (int i = 0; i < services.count(); ++i) {
        QDomElement svc = services.at(i).toElement();
        QString type = svc.elementsByTagName("serviceType").at(0).toElement().text();
        if (type.contains("ContentDirectory")) {
            QString ctrl = svc.elementsByTagName("controlURL").at(0).toElement().text();
            if (!ctrl.startsWith("http"))
                ctrl = baseUrl + (ctrl.startsWith('/') ? ctrl : '/' + ctrl);
            return ctrl;
        }
    }
    return {};
}

QUrl bestIconUrl(const QDomElement &iconList, const QString &baseUrl)
{
    QUrl best;
    int bestScore = -1;

    auto icons = iconList.elementsByTagName("icon");
    for (int i = 0; i < icons.count(); ++i) {
        QDomElement icon = icons.at(i).toElement();
        QString mime   = icon.elementsByTagName("mimetype").at(0).toElement().text();
        QString urlStr = icon.elementsByTagName("url").at(0).toElement().text();
        int     width  = icon.elementsByTagName("width").at(0).toElement().text().toInt();

        if (mime != "image/png" && mime != "image/jpeg" && mime != "image/jpg")
            continue;

        int score = (mime == "image/png") ? 10 : 0;
        score += (width <= 256) ? width : 256 - (width - 256);

        if (score > bestScore) {
            bestScore = score;
            best = QUrl(urlStr.startsWith("http") ? urlStr : baseUrl + urlStr);
        }
    }
    return best;
}

} // namespace DlnaParser
