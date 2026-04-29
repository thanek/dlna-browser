#pragma once
#include <QList>
#include <QString>
#include "dlna/dlnaitem.h"

namespace DlnaUtils {

// Return the nearest non-container row before fromRow, or -1 if none.
int findPrevFile(const QList<DlnaItem> &items, int fromRow);

// Return the nearest non-container row after fromRow, or -1 if none.
int findNextFile(const QList<DlnaItem> &items, int fromRow);

// Format milliseconds as "mm:ss" or "h:mm:ss".
QString formatTime(qint64 ms);

} // namespace DlnaUtils
