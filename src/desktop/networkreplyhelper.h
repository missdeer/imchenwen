#ifndef NETWORKREPLYHELPER_H
#define NETWORKREPLYHELPER_H

#include <QObject>
#include <QNetworkReply>

class NetworkReplyHelper : public QObject
{
    Q_OBJECT
public:
    explicit NetworkReplyHelper(QNetworkReply* reply, QObject *parent = nullptr);
    ~NetworkReplyHelper();
    void waitForFinished();
    QByteArray& content() { return m_content; }
signals:
    void done();
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

    QByteArray m_content;
};


#endif // NETWORKREPLYHELPER_H
