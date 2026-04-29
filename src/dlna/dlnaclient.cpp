#include "dlna/dlnaclient.h"
#include "dlna/dlnaparser.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QUrl>

static const char *BrowseSoapTemplate = R"(<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
            s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
  <s:Body>
    <u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
      <ObjectID>%1</ObjectID>
      <BrowseFlag>BrowseDirectChildren</BrowseFlag>
      <Filter>*</Filter>
      <StartingIndex>0</StartingIndex>
      <RequestedCount>0</RequestedCount>
      <SortCriteria>%2</SortCriteria>
    </u:Browse>
  </s:Body>
</s:Envelope>)";

DlnaClient::DlnaClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void DlnaClient::fetchControlUrl(const QString &location, const QString &serverName)
{
    QNetworkRequest req{QUrl(location)};
    req.setTransferTimeout(5000);
    QNetworkReply *reply = m_nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, location, serverName]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit browseError(reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QString baseStr = QUrl(location).toString(QUrl::RemovePath | QUrl::RemoveQuery);
        QString ctrl = DlnaParser::parseControlUrl(data, baseStr);
        if (!ctrl.isEmpty())
            emit controlUrlReady(serverName, ctrl);
        else
            emit browseError("ContentDirectory service not found");
    });
}

void DlnaClient::browse(const QString &controlUrl,
                         const QString &containerId,
                         const QString &sortCriteria)
{
    QByteArray body = QString(BrowseSoapTemplate).arg(containerId, sortCriteria).toUtf8();

    QNetworkRequest req{QUrl(controlUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"utf-8\"");
    req.setRawHeader("SOAPAction",
        "\"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"");
    req.setTransferTimeout(10000);

    QNetworkReply *reply = m_nam->post(req, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit browseError(reply->errorString());
            return;
        }
        emit browseReady(DlnaParser::parseBrowseResponse(reply->readAll()));
    });
}
