#include "streaminfo.h"

bool operator<(StreamInfoPtr lhs, StreamInfoPtr rhs)
{
    return *(lhs.get()) < *(rhs.get());
}

bool operator<(const StreamInfo &lhs, const StreamInfo &rhs)
{
    QStringList keywords = {
        "144p",
        "210p",
        "240p",
        "640x360",
        "360p",
        "480p",
        "540p",
        "540p H265",
        "640p",
        "1280x720",
        "hd720",
        "720p",
        "720p60",
        "720p H265",
        "hdflv",
        "1080p",
        "1080p60",
        "1440p",
        "2160p",
        "4320p",
        "4k"
    };
    auto it = std::find_if(keywords.rbegin(), keywords.rend(), [lhs](const QString& keyword){
        return lhs.quality.contains(keyword, Qt::CaseInsensitive);
    });
    if (keywords.rend() == it)
        return true;
    auto lhsIndex = std::distance(keywords.rbegin(), it);
    it = std::find_if(keywords.rbegin(), keywords.rend(), [rhs](const QString& keyword){
        return rhs.quality.contains(keyword, Qt::CaseInsensitive);
    });
    if (keywords.rend() == it)
        return false;
    auto rhsIndex = std::distance(keywords.rbegin(), it);
    if (lhsIndex == rhsIndex)
    {
        if (rhs.container.contains("mp4", Qt::CaseInsensitive))
            return true;
        if (lhs.container.contains("mp4", Qt::CaseInsensitive))
            return false;
    }
    return lhsIndex > rhsIndex;
}

bool StreamInfo::maybeAudio()
{
    return (quality.contains("audio only", Qt::CaseInsensitive)
            || quality.contains("DASH audio", Qt::CaseInsensitive)
            || quality.contains("audio/", Qt::CaseInsensitive));
}
