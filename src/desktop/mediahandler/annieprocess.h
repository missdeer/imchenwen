#ifndef ANNIEPROCESS_H
#define ANNIEPROCESS_H

#include "linkresolverprocess.h"

class AnnieProcess : public LinkResolverProcess
{
public:
    explicit AnnieProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject& o, MediaInfoPtr mi);
    void init();
};

#endif // ANNIEPROCESS_H
