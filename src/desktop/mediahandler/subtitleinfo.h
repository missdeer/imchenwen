#ifndef SUBTITLEINFO_H
#define SUBTITLEINFO_H

#include <QSharedPointer>

struct SubtitleInfo
{
    QString language;
    QString url;
    bool manual;
};

typedef QSharedPointer<SubtitleInfo> SubtitleInfoPtr;
typedef QList<SubtitleInfoPtr> Subtitles;


#endif // SUBTITLEINFO_H
