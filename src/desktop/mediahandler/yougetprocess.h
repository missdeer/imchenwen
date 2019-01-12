#ifndef YOUGETPROCESS_H
#define YOUGETPROCESS_H

#include "linkresolverprocess.h"

class YouGetProcess : public LinkResolverProcess
{
public:
    explicit YouGetProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject& o, MediaInfoPtr mi);
    void init();
    void start(const QString& url);
};

#endif // YOUGETPROCESS_H
