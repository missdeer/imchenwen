#ifndef VIPRESOLVER_H
#define VIPRESOLVER_H

#include <QObject>
#include <QNetworkReply>
#include <QSslError>
#include "sniffer.h"

class VIPResolver : public QObject
{
    Q_OBJECT
public:
    explicit VIPResolver(QObject *parent = nullptr);

    void update();
    void resolve(const QString & url);
signals:
    void done(const QStringList&);
    void error();
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onReadFinished();

private:
    QByteArray m_data;
    QStringList m_resolvers;
};

#endif // VIPRESOLVER_H
