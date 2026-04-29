#pragma once
#include <QIcon>
#include <QColor>

// Font Awesome 6 Solid code points used in this project
namespace Fa {
    constexpr uint ArrowLeft    = 0xf060;
    constexpr uint ArrowRight   = 0xf061;
    constexpr uint ArrowUp      = 0xf062;
    constexpr uint House        = 0xf015;
    constexpr uint List         = 0xf03a;
    constexpr uint Grip         = 0xf00a;  // th / grid view
    constexpr uint Sort         = 0xf0dc;
    constexpr uint Folder       = 0xf07b;
    constexpr uint Video        = 0xf03d;
    constexpr uint Music        = 0xf001;
    constexpr uint Image        = 0xf03e;
    constexpr uint File         = 0xf15b;
    constexpr uint Server       = 0xf233;
    constexpr uint Star         = 0xf005;
    constexpr uint StarOutline  = 0xf006;  // star (regular) — also in solid set as empty
    constexpr uint CircleInfo   = 0xf05a;
    constexpr uint Play         = 0xf144;  // circle-play
    constexpr uint SortAlpha    = 0xf15d;
    constexpr uint SortAlphaDesc= 0xf15e;
    constexpr uint SortAmount   = 0xf160;
    constexpr uint Calendar     = 0xf073;
    constexpr uint Minimize     = 0xf068;  // minus — small end of size slider
    constexpr uint Maximize     = 0xf067;  // plus  — large end of size slider
}

class FaIcon {
public:
    // Must be called once before using icon() — loads the font into Qt's font database.
    static void init();

    // Render a Font Awesome code point as a QIcon.
    // size: pixel size of the rendered pixmap (it is square)
    static QIcon icon(uint codePoint,
                      QColor color = Qt::white,
                      int size = 20,
                      double scale = 0.75);

    // Convenience: render into a QPixmap (e.g. for painting directly)
    static QPixmap pixmap(uint codePoint,
                          QColor color,
                          int size,
                          double scale = 0.75);

private:
    static QString s_family;
};
