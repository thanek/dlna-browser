#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QDomDocument>
#include "dlnaitem.h"

struct DlnaServer {
    QString name;
    QString location; // device descriptor URL
    QString usn;
    QUrl iconUrl;
};

class DlnaDiscovery : public QObject {
    Q_OBJECT
public:
    explicit DlnaDiscovery(QObject *parent = nullptr);

    void startDiscovery();

signals:
    void serverFound(const DlnaServer &server);
    void discoveryFinished();

private slots:
    void onReadyRead();
    void onDiscoveryTimeout();

private:
    void fetchDeviceDescriptor(const QString &location, const QString &usn);
    void parseDeviceDescriptor(const QByteArray &data, const QString &location, const QString &usn);

    QUdpSocket *m_socket = nullptr;
    QTimer *m_timer = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
    QSet<QString> m_seenUsn;

    static constexpr quint16 SsdpPort = 1900;
    static const char *SsdpAddress;
};
