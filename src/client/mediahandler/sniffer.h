#ifndef SNIFFER_H
#define SNIFFER_H

#include <QObject>

class Sniffer : public QObject
{
    Q_OBJECT
public:
    explicit Sniffer(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SNIFFER_H