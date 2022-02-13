#ifndef YKDLPROCESS_H
#define YKDLPROCESS_H

#include "linkresolverprocess.h"

class YKDLProcess : public LinkResolverProcess
{
public:
    explicit YKDLProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject &o, MediaInfoPtr mi) override;
    void init() override;
    void start(const QString &url) override;
    void resolved(MediaInfoPtr mi) override;
};

#endif // YKDLPROCESS_H
