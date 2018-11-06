#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QHostAddress>

namespace Util {
    QString secToTime(int second, bool useFormat = false);

    QHostAddress getLocalAddress();
}

#endif // UTIL_H
