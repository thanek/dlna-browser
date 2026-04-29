#pragma once
#include <QObject>
#include <QTest>
#include "dlna/dlnaparser.h"
#include "dlna/dlnautils.h"

class TestDlnaParser : public QObject {
    Q_OBJECT

private slots:
    void parseDidl_emptyString();
    void parseDidl_invalidXml();
    void parseDidl_container();
    void parseDidl_videoItem();
    void parseDidl_audioItem();
    void parseDidl_imageItem();
    void parseDidl_mixedContainersAndItems();
    void parseDidl_thumbnailFallbackToJpegTn();

    void parseBrowseResponse_validEnvelope();
    void parseBrowseResponse_emptyData();
    void parseBrowseResponse_noResultElement();

    void parseControlUrl_found();
    void parseControlUrl_absoluteUrl();
    void parseControlUrl_noContentDirectory();
    void parseControlUrl_emptyData();

    void parseDeviceDescriptor_valid();
    void parseDeviceDescriptor_noFriendlyName();

    void parseDidl_multipleResources_firstWins();
    void parseControlUrl_relativeNoLeadingSlash();

    void bestIconUrl_prefersPngOverJpeg();
    void bestIconUrl_largePngPenalty();
    void bestIconUrl_relativeUrl();
    void bestIconUrl_absoluteUrl();
    void bestIconUrl_emptyList();

    void typeFromMime_video();
    void typeFromMime_audio();
    void typeFromMime_image();
    void typeFromMime_unknown();
    void typeFromMime_empty();
    void isContainer_serverAndContainer();
    void isContainer_mediaTypes();

    void formatTime_zero();
    void formatTime_seconds();
    void formatTime_minutes();
    void formatTime_hours();

    void findNextFile_fromStart();
    void findNextFile_skipsContainer();
    void findNextFile_atEnd();
    void findPrevFile_fromEnd();
    void findPrevFile_skipsContainer();
    void findPrevFile_atStart();
    void findPrevFile_noFiles();
};
