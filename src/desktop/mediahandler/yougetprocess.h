#ifndef YOUGETPROCESS_H
#define YOUGETPROCESS_H

#include "linkresolverprocess.h"

class YouGetProcess : public LinkResolverProcess
{
public:
    explicit YouGetProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject& o, MediaInfoPtr mi);
    void init();
};

#endif // YOUGETPROCESS_H
