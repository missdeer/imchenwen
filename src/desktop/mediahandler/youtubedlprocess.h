#ifndef YOUTUBEDLPROCESS_H
#define YOUTUBEDLPROCESS_H

#include "linkresolverprocess.h"

class YoutubeDLProcess : public LinkResolverProcess
{
public:
    explicit YoutubeDLProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject &o, MediaInfoPtr mi) override;
    void init() override;
    void start(const QString &url) override;
    void resolved(MediaInfoPtr mi) override;

private:
    void parseSubtitle(const QJsonValue& v, MediaInfoPtr mi, bool manual);
};

#endif // YOUTUBEDLPROCESS_H
