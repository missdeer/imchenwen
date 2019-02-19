#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QSharedPointer>
#include "streaminfo.h"
#include "subtitleinfo.h"

struct MediaInfo
{
    QString url;
    QString site;
    QString title;
    Streams ykdl;
    Streams you_get;
    Streams youtube_dl;
    Streams annie;
    Subtitles subtitles;
    int resultCount;
};

typedef QSharedPointer<MediaInfo> MediaInfoPtr;

#endif // MEDIAINFO_H
