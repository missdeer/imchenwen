#ifndef STREAMINFO_H
#define STREAMINFO_H

#include <QSharedPointer>

struct StreamInfo
{
    QString container;
    QString quality;
    QStringList urls;
};

typedef QSharedPointer<StreamInfo> StreamInfoPtr;
typedef QList<StreamInfoPtr> Streams;

bool operator<(const StreamInfo &lhs, const StreamInfo &rhs);
bool operator<(StreamInfoPtr lhs, StreamInfoPtr rhs);

#endif // STREAMINFO_H
