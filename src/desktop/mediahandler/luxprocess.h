#ifndef ANNIEPROCESS_H
#define ANNIEPROCESS_H

#include "linkresolverprocess.h"

class LuxProcess : public LinkResolverProcess
{
public:
    explicit LuxProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject &o, MediaInfoPtr mi) override;
    void init() override;
    void start(const QString &url) override;
    void resolved(MediaInfoPtr mi) override;
};

#endif // ANNIEPROCESS_H
