#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QList>
#include "dlna/dlnaitem.h"

class DlnaClient : public QObject {
    Q_OBJECT
public:
    explicit DlnaClient(QObject *parent = nullptr);

    // Fetch ContentDirectory control URL from device descriptor
    void fetchControlUrl(const QString &location, const QString &serverName);

    // Browse a container; containerId="0" for root
    void browse(const QString &controlUrl,
                const QString &containerId,
                const QString &sortCriteria = QString());

signals:
    void controlUrlReady(const QString &serverName, const QString &controlUrl);
    void browseReady(const QList<DlnaItem> &items);
    void browseError(const QString &message);

private:
    QString buildSoapBody(const QString &containerId, const QString &sortCriteria);

    QNetworkAccessManager *m_nam;
};
