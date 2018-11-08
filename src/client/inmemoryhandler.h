#ifndef INMEMORYHANDLER_H
#define INMEMORYHANDLER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <qhttpengine/handler.h>

class InMemoryHandler : public QHttpEngine::Handler
{
public:
    explicit InMemoryHandler(QObject *parent = nullptr);

    void setM3U8(const QByteArray& m3u8);
    void setReferrer(const QByteArray& referrer);
    void setUserAgent(const QByteArray& userAgent);
    QString mapUrl(const QString& url);
    QString mapUrl(const QStringList& urls);
    void clear();
protected:

    /**
     * @brief Reimplementation of [Handler::process()](QHttpEngine::Handler::process)
     */
    virtual void process(QHttpEngine::Socket *socket, const QString &path);
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onUniqueMediaReadFinished();
    void onMultiMediaReadFinished();
private:
    QByteArray m_m3u8;
    QByteArray m_referrer;
    QByteArray m_userAgent;
    QString m_localAddress;
    QNetworkAccessManager m_nam;
    QMap<QNetworkReply*, QHttpEngine::Socket *> m_replySocketMap;
    QSet<QNetworkReply*> m_headerWritten;
    QMap<QString, QString> m_1to1UrlMap;
    QMap<QString, QStringList> m_1toNUrlMap;
    void returnMediaM3U8(QHttpEngine::Socket *socket);
    void relayMedia(QHttpEngine::Socket *socket, const QString& url);
    void relayMedia(QHttpEngine::Socket *socket, const QStringList& urls, int index);
    void requestRelayMedia(QHttpEngine::Socket *socket, const QString& url);
};

#endif // INMEMORYHANDLER_H
