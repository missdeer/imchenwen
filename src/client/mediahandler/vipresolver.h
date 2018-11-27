#ifndef VIPRESOLVER_H
#define VIPRESOLVER_H

#include <QObject>

class VIPResolver : public QObject
{
    Q_OBJECT
public:
    explicit VIPResolver(QObject *parent = nullptr);

signals:

public slots:
};

#endif // VIPRESOLVER_H