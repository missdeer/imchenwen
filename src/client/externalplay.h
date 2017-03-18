#ifndef EXTERNALPLAY_H
#define EXTERNALPLAY_H

#include "linkresolver.h"
#include <QProcess>

class ExternalPlay
{
public:
    ExternalPlay();
    void Play(const Streams& streams);

private:
    QProcess& process();
};

#endif // EXTERNALPLAY_H
