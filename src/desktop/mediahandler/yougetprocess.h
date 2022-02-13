#ifndef YOUGETPROCESS_H
#define YOUGETPROCESS_H

#include "linkresolverprocess.h"

class YouGetProcess : public LinkResolverProcess
{
public:
    explicit YouGetProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject &o, MediaInfoPtr mi) override;
    void init() override;
    void start(const QString &url) override;
    void resolved(MediaInfoPtr mi) override;
};

#endif // YOUGETPROCESS_H
