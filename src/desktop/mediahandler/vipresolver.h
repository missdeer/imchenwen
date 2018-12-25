#ifndef VIPRESOLVER_H
#define VIPRESOLVER_H

#include <QObject>
#include <QNetworkReply>
#include <QSslError>

class Sniffer;

class VIPResolver : public QObject
{
    Q_OBJECT
public:
    explicit VIPResolver(QObject *parent = nullptr);
    ~VIPResolver();

    void update();
    void resolve(const QString & url);
    void stop();
    bool ready() const { return m_ready; }
signals:
    void done(const QString&, const QStringList&);
    void error();
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onReadFinished();
    void onSnifferDone(const QString& originalUrl, const QString &result);
    void onSnifferError();
private:
    bool m_ready;
    bool m_stopped;
    int m_resolverIndex;
    int m_finishedCount;
    QByteArray m_data;
    QStringList m_resolvers;
    QStringList m_results;
    QList<Sniffer*> m_sniffers;
    QString m_lastResolveUrl;
    bool doSniff(Sniffer *sniffer);
    void continueSniff(Sniffer *sniffer);
};

#endif // VIPRESOLVER_H
