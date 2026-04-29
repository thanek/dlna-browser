#include "dlna/dlnautils.h"

namespace DlnaUtils {

int findPrevFile(const QList<DlnaItem> &items, int fromRow)
{
    for (int row = fromRow - 1; row >= 0; --row)
        if (!items.at(row).isContainer())
            return row;
    return -1;
}

int findNextFile(const QList<DlnaItem> &items, int fromRow)
{
    for (int row = fromRow + 1; row < items.size(); ++row)
        if (!items.at(row).isContainer())
            return row;
    return -1;
}

QString formatTime(qint64 ms)
{
    qint64 s = ms / 1000;
    qint64 h = s / 3600;
    qint64 m = (s % 3600) / 60;
    s = s % 60;
    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

} // namespace DlnaUtils
