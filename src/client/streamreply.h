#ifndef STREAMREPLY_H
#define STREAMREPLY_H

#include <QObject>
#include <QNetworkReply>
#include <QSharedPointer>

class StreamReply : public QObject
{
    Q_OBJECT
public:
    StreamReply(int index, QNetworkReply* reply, QObject *parent = 0);
    ~StreamReply();
    void stop();
    int index() const;

signals:
    void done();
    void cancel();
    void errorMessage(QNetworkReply::NetworkError , QString);
public slots:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();
    void sslErrors(const QList<QSslError> & errors);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void readyRead();
private:
    QNetworkReply* m_reply;
    int m_index;
};

typedef QSharedPointer<StreamReply> StreamReplyPtr;

#endif // STREAMREPLY_H
