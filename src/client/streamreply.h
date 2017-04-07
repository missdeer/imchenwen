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
    QByteArray read();
    bool atEnd();
    const QList<QNetworkReply::RawHeaderPair> &rawHeaderPairs() const;
    int index() const;

    int statusCode() const;

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
    QFile* m_in;
    QFile* m_out;
    QString m_cachePath;
    int m_index;
    int m_statusCode;
    bool m_finished;
};

typedef QSharedPointer<StreamReply> StreamReplyPtr;

#endif // STREAMREPLY_H
