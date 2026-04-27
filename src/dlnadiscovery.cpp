#include "dlnadiscovery.h"
#include "dlnaparser.h"

#include <QNetworkRequest>
#include <QHostAddress>
#include <QUrl>

const char *DlnaDiscovery::SsdpAddress = "239.255.255.250";

static const char *SsdpSearch =
    "M-SEARCH * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "MX: 3\r\n"
    "ST: urn:schemas-upnp-org:device:MediaServer:1\r\n"
    "\r\n";

DlnaDiscovery::DlnaDiscovery(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_timer(new QTimer(this))
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_socket, &QUdpSocket::readyRead, this, &DlnaDiscovery::onReadyRead);
    connect(m_timer, &QTimer::timeout, this, &DlnaDiscovery::onDiscoveryTimeout);
    m_timer->setSingleShot(true);
}

void DlnaDiscovery::startDiscovery()
{
    m_seenUsn.clear();
    m_socket->close();
    m_socket->bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress);

    QByteArray msg(SsdpSearch);
    m_socket->writeDatagram(msg, QHostAddress(SsdpAddress), SsdpPort);

    m_timer->start(5000);
}

void DlnaDiscovery::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(data.data(), data.size());

        QString response(data);
        QString location, usn;

        for (const QString &line : response.split("\r\n")) {
            if (line.startsWith("LOCATION:", Qt::CaseInsensitive))
                location = line.mid(9).trimmed();
            else if (line.startsWith("USN:", Qt::CaseInsensitive))
                usn = line.mid(4).trimmed();
        }

        if (!location.isEmpty() && !usn.isEmpty() && !m_seenUsn.contains(usn)) {
            m_seenUsn.insert(usn);
            fetchDeviceDescriptor(location, usn);
        }
    }
}

void DlnaDiscovery::onDiscoveryTimeout()
{
    m_socket->close();
    emit discoveryFinished();
}

void DlnaDiscovery::fetchDeviceDescriptor(const QString &location, const QString &usn)
{
    QNetworkRequest req{QUrl(location)};
    req.setTransferTimeout(5000);
    QNetworkReply *reply = m_nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, location, usn]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError)
            parseDeviceDescriptor(reply->readAll(), location, usn);
    });
}

void DlnaDiscovery::parseDeviceDescriptor(const QByteArray &data,
                                           const QString &location,
                                           const QString &usn)
{
    DlnaServer server = DlnaParser::parseDeviceDescriptor(data, location, usn);
    if (server.name.isEmpty()) return;
    emit serverFound(server);
}
