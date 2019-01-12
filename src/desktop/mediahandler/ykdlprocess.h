#ifndef YKDLPROCESS_H
#define YKDLPROCESS_H

#include "linkresolverprocess.h"

class YKDLProcess : public LinkResolverProcess
{
public:
    explicit YKDLProcess(QObject *parent = nullptr);
    void parseNode(const QJsonObject& o, MediaInfoPtr mi);
    void init();
};

#endif // YKDLPROCESS_H
