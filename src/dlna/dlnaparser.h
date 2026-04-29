#pragma once
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QList>
#include "dlna/dlnaitem.h"
#include "dlna/dlnadiscovery.h"  // DlnaServer

// Pure XML-parsing functions extracted from DlnaClient / DlnaDiscovery.
// All methods are stateless and take raw data in, return value types out —
// easy to unit-test without network or Qt event loop.
namespace DlnaParser {

// Parse a SOAP Browse response and return the list of DLNA items.
// Returns an empty list on XML errors.
QList<DlnaItem> parseBrowseResponse(const QByteArray &soapResponse);

// Parse just the inner DIDL-Lite XML string (the unescaped <Result> content).
QList<DlnaItem> parseDidl(const QString &didlXml);

// Extract the ContentDirectory SOAP control URL from a device descriptor XML.
// `baseUrl` is used to resolve relative paths (e.g. "http://192.168.1.1:8080").
// Returns an empty string if not found.
QString parseControlUrl(const QByteArray &descriptorXml, const QString &baseUrl);

// Extract server metadata from a device descriptor XML.
// Returns a default-constructed DlnaServer (empty name) on failure.
DlnaServer parseDeviceDescriptor(const QByteArray &descriptorXml,
                                  const QString &location,
                                  const QString &usn);

// Select the best icon URL from a parsed <iconList> element.
QUrl bestIconUrl(const QDomElement &iconList, const QString &baseUrl);

} // namespace DlnaParser
