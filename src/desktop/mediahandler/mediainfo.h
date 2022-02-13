#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QSharedPointer>

#include "streaminfo.h"
#include "subtitleinfo.h"

struct MediaInfo
{
    QString   url;
    QString   site;
    QString   title;
    Streams   ykdl;
    Streams   you_get;
    Streams   youtube_dl;
    Streams   lux;
    bool      ykdlDone {false};
    bool      you_getDone {false};
    bool      youtube_dlDone {false};
    bool      luxDone {false};
    Subtitles subtitles;
    int       resultCount;
    void      Reset()
    {
        url.clear();
        site.clear();
        title.clear();
        ykdl.clear();
        you_get.clear();
        youtube_dl.clear();
        lux.clear();
        ykdlDone       = false;
        you_getDone    = false;
        youtube_dlDone = false;
        luxDone        = false;
        subtitles.clear();
        resultCount = 0;
    }
};

typedef QSharedPointer<MediaInfo> MediaInfoPtr;

#endif // MEDIAINFO_H
