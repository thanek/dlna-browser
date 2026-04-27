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
    src/faicon.cpp
    src/faicon.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/dlnadiscovery.cpp
    src/dlnadiscovery.h
    src/dlnaclient.cpp
    src/dlnaclient.h
    src/dlnaitem.h
    src/dlnaparser.cpp
    src/dlnaparser.h
    src/dlnamodel.cpp
    src/dlnamodel.h
    src/contentview.cpp
    src/contentview.h
    src/favoritespanel.cpp
    src/favoritespanel.h
    src/addressbar.cpp
    src/addressbar.h
    src/mediaviewer.cpp
    src/mediaviewer.h
    src/videowidget.cpp
    src/videowidget.h
    src/imagewidget.cpp
    src/imagewidget.h
    src/infowidget.cpp
    src/infowidget.h
    resources/resources.qrc
)

qt_add_executable(DlnaBrowser ${SOURCES})

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
