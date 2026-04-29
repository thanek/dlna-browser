#include "test_dlnaparser.h"

// ---------------------------------------------------------------------------
// Helper: build a SOAP BrowseResponse with DIDL content as the Result element
// ---------------------------------------------------------------------------
static QByteArray soapEnvelope(const QString &didlXml)
{
    QString escaped = didlXml;
    escaped.replace('&', "&amp;");
    escaped.replace('<', "&lt;");
    escaped.replace('>', "&gt;");
    escaped.replace('"', "&quot;");

    return (QString(
        "<?xml version=\"1.0\"?>"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        "  <s:Body>"
        "    <u:BrowseResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
        "      <Result>%1</Result>"
        "      <NumberReturned>1</NumberReturned>"
        "    </u:BrowseResponse>"
        "  </s:Body>"
        "</s:Envelope>").arg(escaped)).toUtf8();
}

// ─── DIDL-Lite fixtures ───────────────────────────────────────────────────────

static const char *DidlContainer = R"(<?xml version="1.0"?>
<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
           xmlns:dc="http://purl.org/dc/elements/1.1/"
           xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">
  <container id="5" parentID="0" childCount="3" restricted="1">
    <dc:title>Movies</dc:title>
  </container>
</DIDL-Lite>)";

static const char *DidlVideoItem = R"(<?xml version="1.0"?>
<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
           xmlns:dc="http://purl.org/dc/elements/1.1/"
           xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">
  <item id="42" parentID="5" restricted="1">
    <dc:title>My Video</dc:title>
    <dc:date>2024-01-15</dc:date>
    <upnp:albumArtURI>http://server/art.jpg</upnp:albumArtURI>
    <res protocolInfo="http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4">http://server/video.mp4</res>
    <res protocolInfo="http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN">http://server/thumb.jpg</res>
  </item>
</DIDL-Lite>)";

static const char *DidlAudioItem = R"(<?xml version="1.0"?>
<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
           xmlns:dc="http://purl.org/dc/elements/1.1/"
           xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">
  <item id="7" parentID="0" restricted="1">
    <dc:title>Great Song</dc:title>
    <res protocolInfo="http-get:*:audio/mpeg:DLNA.ORG_PN=MP3">http://server/song.mp3</res>
  </item>
</DIDL-Lite>)";

static const char *DidlMixed = R"(<?xml version="1.0"?>
<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
           xmlns:dc="http://purl.org/dc/elements/1.1/"
           xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/">
  <container id="1" parentID="0" childCount="0" restricted="1">
    <dc:title>Folder A</dc:title>
  </container>
  <item id="10" parentID="0" restricted="1">
    <dc:title>Photo</dc:title>
    <res protocolInfo="http-get:*:image/jpeg:*">http://server/photo.jpg</res>
  </item>
  <container id="2" parentID="0" childCount="0" restricted="1">
    <dc:title>Folder B</dc:title>
  </container>
  <item id="11" parentID="0" restricted="1">
    <dc:title>Video</dc:title>
    <res protocolInfo="http-get:*:video/avi:*">http://server/video.avi</res>
  </item>
</DIDL-Lite>)";

static const char *DeviceDescriptor = R"(<?xml version="1.0"?>
<root xmlns="urn:schemas-upnp-org:device-1-0">
  <device>
    <friendlyName>My Media Server</friendlyName>
    <serviceList>
      <service>
        <serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>
        <controlURL>/AVTransport/control</controlURL>
      </service>
      <service>
        <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>
        <serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId>
        <controlURL>/ContentDirectory/control</controlURL>
      </service>
    </serviceList>
    <iconList>
      <icon>
        <mimetype>image/jpeg</mimetype>
        <width>48</width>
        <url>/icons/small.jpg</url>
      </icon>
      <icon>
        <mimetype>image/png</mimetype>
        <width>120</width>
        <url>/icons/medium.png</url>
      </icon>
      <icon>
        <mimetype>image/png</mimetype>
        <width>512</width>
        <url>/icons/large.png</url>
      </icon>
    </iconList>
  </device>
</root>)";

// ─── parseDidl ────────────────────────────────────────────────────────────────

void TestDlnaParser::parseDidl_emptyString()
{
    QVERIFY(DlnaParser::parseDidl({}).isEmpty());
}

void TestDlnaParser::parseDidl_invalidXml()
{
    QVERIFY(DlnaParser::parseDidl("<not valid xml <><>").isEmpty());
}

void TestDlnaParser::parseDidl_container()
{
    auto items = DlnaParser::parseDidl(QString::fromUtf8(DidlContainer));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].id, "5");
    QCOMPARE(items[0].title, "Movies");
    QCOMPARE(items[0].type, DlnaItemType::Container);
    QVERIFY(items[0].isContainer());
}

void TestDlnaParser::parseDidl_videoItem()
{
    auto items = DlnaParser::parseDidl(QString::fromUtf8(DidlVideoItem));
    QCOMPARE(items.size(), 1);
    const DlnaItem &it = items[0];
    QCOMPARE(it.id, "42");
    QCOMPARE(it.title, "My Video");
    QCOMPARE(it.type, DlnaItemType::Video);
    QCOMPARE(it.mimeType, "video/mp4");
    QCOMPARE(it.resourceUrl, QUrl("http://server/video.mp4"));
    QCOMPARE(it.date, "2024-01-15");
    QCOMPARE(it.thumbnailUrl, QUrl("http://server/art.jpg")); // albumArtURI takes priority
    QVERIFY(!it.isContainer());
}

void TestDlnaParser::parseDidl_audioItem()
{
    auto items = DlnaParser::parseDidl(QString::fromUtf8(DidlAudioItem));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].type, DlnaItemType::Audio);
    QCOMPARE(items[0].mimeType, "audio/mpeg");
}

void TestDlnaParser::parseDidl_imageItem()
{
    const char *didl =
        "<?xml version=\"1.0\"?>"
        "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\""
        "           xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
        "  <item id=\"9\" parentID=\"0\" restricted=\"1\">"
        "    <dc:title>Sunset</dc:title>"
        "    <res protocolInfo=\"http-get:*:image/png:*\">http://server/sunset.png</res>"
        "  </item>"
        "</DIDL-Lite>";
    auto items = DlnaParser::parseDidl(QString::fromUtf8(didl));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].type, DlnaItemType::Image);
}

void TestDlnaParser::parseDidl_mixedContainersAndItems()
{
    auto items = DlnaParser::parseDidl(QString::fromUtf8(DidlMixed));
    // Containers are collected first (elementsByTagName("container") first loop)
    QCOMPARE(items.size(), 4);
    QVERIFY(items[0].isContainer()); // Folder A
    QVERIFY(items[1].isContainer()); // Folder B
    QVERIFY(!items[2].isContainer()); // Photo
    QVERIFY(!items[3].isContainer()); // Video
}

void TestDlnaParser::parseDidl_thumbnailFallbackToJpegTn()
{
    const char *didl =
        "<?xml version=\"1.0\"?>"
        "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\""
        "           xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
        "  <item id=\"1\" parentID=\"0\" restricted=\"1\">"
        "    <dc:title>Clip</dc:title>"
        "    <res protocolInfo=\"http-get:*:video/mp4:*\">http://server/clip.mp4</res>"
        "    <res protocolInfo=\"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN\">http://server/tn.jpg</res>"
        "  </item>"
        "</DIDL-Lite>";
    auto items = DlnaParser::parseDidl(QString::fromUtf8(didl));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].resourceUrl, QUrl("http://server/clip.mp4"));
    QCOMPARE(items[0].thumbnailUrl, QUrl("http://server/tn.jpg"));
}

// ─── parseBrowseResponse ──────────────────────────────────────────────────────

void TestDlnaParser::parseBrowseResponse_validEnvelope()
{
    auto items = DlnaParser::parseBrowseResponse(soapEnvelope(QString::fromUtf8(DidlAudioItem)));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].title, "Great Song");
}

void TestDlnaParser::parseBrowseResponse_emptyData()
{
    QVERIFY(DlnaParser::parseBrowseResponse({}).isEmpty());
}

void TestDlnaParser::parseBrowseResponse_noResultElement()
{
    const QByteArray fault =
        "<?xml version=\"1.0\"?>"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        "  <s:Body><s:Fault><faultstring>UPnPError</faultstring></s:Fault></s:Body>"
        "</s:Envelope>";
    QVERIFY(DlnaParser::parseBrowseResponse(fault).isEmpty());
}

// ─── parseDidl — edge cases ───────────────────────────────────────────────────

void TestDlnaParser::parseDidl_multipleResources_firstWins()
{
    // First non-thumbnail <res> wins; subsequent ones are ignored.
    const char *didl =
        "<?xml version=\"1.0\"?>"
        "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\""
        "           xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
        "  <item id=\"1\" parentID=\"0\" restricted=\"1\">"
        "    <dc:title>Clip</dc:title>"
        "    <res protocolInfo=\"http-get:*:video/mp4:*\">http://server/hd.mp4</res>"
        "    <res protocolInfo=\"http-get:*:video/mpeg:*\">http://server/sd.mpg</res>"
        "    <res protocolInfo=\"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN\">http://server/tn.jpg</res>"
        "  </item>"
        "</DIDL-Lite>";
    auto items = DlnaParser::parseDidl(QString::fromUtf8(didl));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].resourceUrl, QUrl("http://server/hd.mp4"));
    QCOMPARE(items[0].mimeType, "video/mp4");
    QCOMPARE(items[0].thumbnailUrl, QUrl("http://server/tn.jpg"));
}

// ─── parseControlUrl ─────────────────────────────────────────────────────────

void TestDlnaParser::parseControlUrl_found()
{
    QString ctrl = DlnaParser::parseControlUrl(QByteArray(DeviceDescriptor),
                                                "http://192.168.1.1:8080");
    QCOMPARE(ctrl, "http://192.168.1.1:8080/ContentDirectory/control");
}

void TestDlnaParser::parseControlUrl_absoluteUrl()
{
    const char *xml =
        "<?xml version=\"1.0\"?><root>"
        "  <serviceList><service>"
        "    <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>"
        "    <controlURL>http://other.host/ctrl</controlURL>"
        "  </service></serviceList>"
        "</root>";
    QCOMPARE(DlnaParser::parseControlUrl(QByteArray(xml), "http://192.168.1.1:8080"),
             "http://other.host/ctrl");
}

void TestDlnaParser::parseControlUrl_noContentDirectory()
{
    const char *xml =
        "<?xml version=\"1.0\"?><root>"
        "  <serviceList><service>"
        "    <serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>"
        "    <controlURL>/av/ctrl</controlURL>"
        "  </service></serviceList>"
        "</root>";
    QVERIFY(DlnaParser::parseControlUrl(QByteArray(xml), "http://192.168.1.1:8080").isEmpty());
}

void TestDlnaParser::parseControlUrl_emptyData()
{
    QVERIFY(DlnaParser::parseControlUrl({}, "http://192.168.1.1:8080").isEmpty());
}

void TestDlnaParser::parseControlUrl_relativeNoLeadingSlash()
{
    const char *xml =
        "<?xml version=\"1.0\"?><root>"
        "  <serviceList><service>"
        "    <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>"
        "    <controlURL>cd/control</controlURL>"
        "  </service></serviceList>"
        "</root>";
    QCOMPARE(DlnaParser::parseControlUrl(QByteArray(xml), "http://192.168.1.1:8080"),
             "http://192.168.1.1:8080/cd/control");
}

// ─── parseDeviceDescriptor ────────────────────────────────────────────────────

void TestDlnaParser::parseDeviceDescriptor_valid()
{
    DlnaServer srv = DlnaParser::parseDeviceDescriptor(
        QByteArray(DeviceDescriptor),
        "http://192.168.1.1:8080/desc.xml",
        "uuid:1234::urn:schemas-upnp-org:device:MediaServer:1");
    QCOMPARE(srv.name, "My Media Server");
    QCOMPARE(srv.location, "http://192.168.1.1:8080/desc.xml");
    QCOMPARE(srv.usn, "uuid:1234::urn:schemas-upnp-org:device:MediaServer:1");
    QVERIFY(!srv.iconUrl.isEmpty());
}

void TestDlnaParser::parseDeviceDescriptor_noFriendlyName()
{
    const char *xml = "<?xml version=\"1.0\"?><root><device></device></root>";
    DlnaServer srv = DlnaParser::parseDeviceDescriptor(QByteArray(xml), "http://x", "usn");
    QVERIFY(srv.name.isEmpty());
}

// ─── formatTime ──────────────────────────────────────────────────────────────

void TestDlnaParser::formatTime_zero()
{
    QCOMPARE(DlnaUtils::formatTime(0), "00:00");
}

void TestDlnaParser::formatTime_seconds()
{
    QCOMPARE(DlnaUtils::formatTime(5000),  "00:05");
    QCOMPARE(DlnaUtils::formatTime(59000), "00:59");
}

void TestDlnaParser::formatTime_minutes()
{
    QCOMPARE(DlnaUtils::formatTime(60000),   "01:00");
    QCOMPARE(DlnaUtils::formatTime(90000),   "01:30");
    QCOMPARE(DlnaUtils::formatTime(3599000), "59:59");
}

void TestDlnaParser::formatTime_hours()
{
    QCOMPARE(DlnaUtils::formatTime(3600000), "1:00:00");
    QCOMPARE(DlnaUtils::formatTime(3661000), "1:01:01");
    QCOMPARE(DlnaUtils::formatTime(7322000), "2:02:02");
}

// ─── findPrevFile / findNextFile ──────────────────────────────────────────────

static QList<DlnaItem> makeItemList()
{
    // Layout: [Container, File, Container, File, File]  (indices 0-4)
    QList<DlnaItem> items;
    auto add = [&](DlnaItemType t) {
        DlnaItem it; it.type = t; items.append(it);
    };
    add(DlnaItemType::Container); // 0
    add(DlnaItemType::Video);     // 1
    add(DlnaItemType::Container); // 2
    add(DlnaItemType::Audio);     // 3
    add(DlnaItemType::Image);     // 4
    return items;
}

void TestDlnaParser::findNextFile_fromStart()
{
    QCOMPARE(DlnaUtils::findNextFile(makeItemList(), 0), 1);
}

void TestDlnaParser::findNextFile_skipsContainer()
{
    QCOMPARE(DlnaUtils::findNextFile(makeItemList(), 1), 3);
}

void TestDlnaParser::findNextFile_atEnd()
{
    QCOMPARE(DlnaUtils::findNextFile(makeItemList(), 4), -1);
}

void TestDlnaParser::findPrevFile_fromEnd()
{
    QCOMPARE(DlnaUtils::findPrevFile(makeItemList(), 4), 3);
}

void TestDlnaParser::findPrevFile_skipsContainer()
{
    QCOMPARE(DlnaUtils::findPrevFile(makeItemList(), 3), 1);
}

void TestDlnaParser::findPrevFile_atStart()
{
    QCOMPARE(DlnaUtils::findPrevFile(makeItemList(), 1), -1);
}

void TestDlnaParser::findPrevFile_noFiles()
{
    QList<DlnaItem> items;
    DlnaItem c; c.type = DlnaItemType::Container;
    items << c << c;
    QCOMPARE(DlnaUtils::findPrevFile(items, 1), -1);
    QCOMPARE(DlnaUtils::findNextFile(items, 0), -1);
}

// ─── bestIconUrl ─────────────────────────────────────────────────────────────

static QDomElement parseIconList(const QByteArray &xml)
{
    QDomDocument doc;
    doc.setContent(xml);
    return doc.documentElement();
}

void TestDlnaParser::bestIconUrl_prefersPngOverJpeg()
{
    // PNG 128 (score 138) beats JPEG 128 (score 128)
    auto el = parseIconList(
        "<iconList>"
        "  <icon><mimetype>image/jpeg</mimetype><width>128</width><url>/icon.jpg</url></icon>"
        "  <icon><mimetype>image/png</mimetype><width>128</width><url>/icon.png</url></icon>"
        "</iconList>");
    QCOMPARE(DlnaParser::bestIconUrl(el, "http://host").toString(),
             "http://host/icon.png");
}

void TestDlnaParser::bestIconUrl_largePngPenalty()
{
    // PNG 512 (score 10) loses to JPEG 200 (score 200)
    auto el = parseIconList(
        "<iconList>"
        "  <icon><mimetype>image/png</mimetype><width>512</width><url>/large.png</url></icon>"
        "  <icon><mimetype>image/jpeg</mimetype><width>200</width><url>/medium.jpg</url></icon>"
        "</iconList>");
    QCOMPARE(DlnaParser::bestIconUrl(el, "http://host").toString(),
             "http://host/medium.jpg");
}

void TestDlnaParser::bestIconUrl_relativeUrl()
{
    auto el = parseIconList(
        "<iconList>"
        "  <icon><mimetype>image/png</mimetype><width>64</width><url>/icons/icon.png</url></icon>"
        "</iconList>");
    QCOMPARE(DlnaParser::bestIconUrl(el, "http://192.168.1.1:8080").toString(),
             "http://192.168.1.1:8080/icons/icon.png");
}

void TestDlnaParser::bestIconUrl_absoluteUrl()
{
    auto el = parseIconList(
        "<iconList>"
        "  <icon><mimetype>image/png</mimetype><width>64</width><url>http://other.host/icon.png</url></icon>"
        "</iconList>");
    QCOMPARE(DlnaParser::bestIconUrl(el, "http://192.168.1.1:8080").toString(),
             "http://other.host/icon.png");
}

void TestDlnaParser::bestIconUrl_emptyList()
{
    auto el = parseIconList("<iconList></iconList>");
    QVERIFY(DlnaParser::bestIconUrl(el, "http://host").isEmpty());
}

// ─── DlnaItem helpers ─────────────────────────────────────────────────────────

void TestDlnaParser::typeFromMime_video()
{
    QCOMPARE(DlnaItem::typeFromMime("video/mp4"),       DlnaItemType::Video);
    QCOMPARE(DlnaItem::typeFromMime("video/x-matroska"),DlnaItemType::Video);
}

void TestDlnaParser::typeFromMime_audio()
{
    QCOMPARE(DlnaItem::typeFromMime("audio/mpeg"), DlnaItemType::Audio);
    QCOMPARE(DlnaItem::typeFromMime("audio/flac"), DlnaItemType::Audio);
}

void TestDlnaParser::typeFromMime_image()
{
    QCOMPARE(DlnaItem::typeFromMime("image/jpeg"), DlnaItemType::Image);
    QCOMPARE(DlnaItem::typeFromMime("image/png"),  DlnaItemType::Image);
}

void TestDlnaParser::typeFromMime_unknown()
{
    QCOMPARE(DlnaItem::typeFromMime("application/pdf"), DlnaItemType::Unknown);
    QCOMPARE(DlnaItem::typeFromMime("text/plain"),      DlnaItemType::Unknown);
}

void TestDlnaParser::typeFromMime_empty()
{
    QCOMPARE(DlnaItem::typeFromMime(""), DlnaItemType::Unknown);
}

void TestDlnaParser::isContainer_serverAndContainer()
{
    DlnaItem srv; srv.type = DlnaItemType::Server;
    QVERIFY(srv.isContainer());
    DlnaItem folder; folder.type = DlnaItemType::Container;
    QVERIFY(folder.isContainer());
}

void TestDlnaParser::isContainer_mediaTypes()
{
    for (auto t : {DlnaItemType::Video, DlnaItemType::Audio,
                   DlnaItemType::Image, DlnaItemType::Unknown}) {
        DlnaItem it; it.type = t;
        QVERIFY(!it.isContainer());
    }
}

QTEST_APPLESS_MAIN(TestDlnaParser)
