#ifndef ANNIEPROCESS_H
#define ANNIEPROCESS_H

#include "linkresolverprocess.h"

class LuxProcess : public LinkResolverProcess
{
public:
    explicit LuxProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject& o, MediaInfoPtr mi);
    void init();
    void start(const QString& url);
};

#endif // ANNIEPROCESS_H
