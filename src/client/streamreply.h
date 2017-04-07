#ifndef STREAMREPLY_H
#define STREAMREPLY_H

#include <QObject>
#include <QNetworkReply>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE

class StreamReply : public QObject
{
    Q_OBJECT
public:
    StreamReply(int index, QNetworkReply* reply, QObject *parent = 0);
    ~StreamReply();
    void stop();
signals:
    void localReadyRead();
    void done();
    void cancel();
    void errorMessage(QNetworkReply::NetworkError , QString);
public slots:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();
    void sslErrors(const QList<QSslError> & errors);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void remoteReadyRead();
private:
    QNetworkReply* m_reply;
    QFile* m_in;
    QString m_cachePath;
    bool m_finished;
    bool m_localReadyRead;
};

typedef QSharedPointer<StreamReply> StreamReplyPtr;

#endif // STREAMREPLY_H
