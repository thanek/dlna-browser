cmake_minimum_required(VERSION 3.20)
project(DlnaBrowser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Xml Multimedia MultimediaWidgets)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    # dlna — protocol layer
    src/dlna/dlnaitem.h
    src/dlna/dlnadiscovery.h
    src/dlna/dlnadiscovery.cpp
    src/dlna/dlnaclient.h
    src/dlna/dlnaclient.cpp
    src/dlna/dlnaparser.h
    src/dlna/dlnaparser.cpp
    src/dlna/dlnautils.h
    src/dlna/dlnautils.cpp
    # ui — shared UI utilities
    src/ui/faicon.h
    src/ui/faicon.cpp
    # browser — main browser window
    src/browser/dlnaicons.h
    src/browser/dlnamodel.h
    src/browser/dlnamodel.cpp
    src/browser/contentview.h
    src/browser/contentview.cpp
    src/browser/addressbar.h
    src/browser/addressbar.cpp
    src/browser/favoritespanel.h
    src/browser/favoritespanel.cpp
    src/browser/mainwindow.h
    src/browser/mainwindow.cpp
    # mediaviewer — media playback
    src/mediaviewer/mediawidget.h
    src/mediaviewer/mediawidget.cpp
    src/mediaviewer/videowidget.h
    src/mediaviewer/videowidget.cpp
    src/mediaviewer/imagewidget.h
    src/mediaviewer/imagewidget.cpp
    src/mediaviewer/infowidget.h
    src/mediaviewer/infowidget.cpp
    src/mediaviewer/mediaviewer.h
    src/mediaviewer/mediaviewer.cpp
    resources/resources.qrc
)

qt_add_executable(DlnaBrowser ${SOURCES})
target_include_directories(DlnaBrowser PRIVATE src/)

target_link_libraries(DlnaBrowser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

if(WIN32)
    set_target_properties(DlnaBrowser PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(DlnaBrowser PRIVATE ws2_32)
endif()

if(APPLE)
    set_target_properties(DlnaBrowser PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "DLNA Browser"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

install(TARGETS DlnaBrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

enable_testing()
add_subdirectory(tests)
