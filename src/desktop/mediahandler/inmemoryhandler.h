#ifndef INMEMORYHANDLER_H
#define INMEMORYHANDLER_H

#include <QNetworkReply>
#include <QSslError>
#include <QUdpSocket>
#include <QQueue>
#include <qhttpengine/handler.h>

class InMemoryHandler : public QHttpEngine::Handler
{
    Q_OBJECT
public:
    explicit InMemoryHandler(QObject *parent = nullptr);

    void setM3U8(const QByteArray& m3u8);
    void setReferrer(const QByteArray& referrer);
    void setUserAgent(const QByteArray& userAgent);
    QString mapUrl(const QString& url);
    void clear();
    virtual void process(QHttpEngine::Socket *socket, const QString &path);
    void newMediaData(const QByteArray &data);
signals:

public slots:
    void onInputEnd();
private slots:
    void onNetworkError(QNetworkReply::NetworkError code);
    void onNetworkSSLErrors(const QList<QSslError> &errors);
    void onReadyRead();
    void onMediaReadFinished();
    void relayMediaData();
private:
    bool m_inputEnd;
    QByteArray m_m3u8;
    QByteArray m_mediaData;
    QByteArray m_referrer;
    QByteArray m_userAgent;
    QString m_localAddress;
    QMap<QNetworkReply*, QHttpEngine::Socket *> m_replySocketMap;
    QSet<QNetworkReply*> m_headerWritten;
    QMap<QString, QString> m_1to1UrlMap;
    QHttpEngine::Socket *m_mediaSocket;
    void returnMediaM3U8(QHttpEngine::Socket *socket);
    void returnMediaData(QHttpEngine::Socket *socket);
    void relayMedia(QHttpEngine::Socket *socket, const QString& url);
    void serveFileSystemFile(QHttpEngine::Socket* socket, const QString &absolutePath);
};

#endif // INMEMORYHANDLER_H
