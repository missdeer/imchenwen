#ifndef VIPRESOLVER_H
#define VIPRESOLVER_H

#include <QObject>
#include "sniffer.h"

class VIPResolver : public QObject
{
    Q_OBJECT
public:
    explicit VIPResolver(QObject *parent = nullptr);

    void resolve(const QString & url);
signals:
    void done(const QStringList&);
    void error();
public slots:
};

#endif // VIPRESOLVER_H
