#include "streamreply.h"

StreamReply::StreamReply(int index, QNetworkReply *reply, QObject *parent)
    : QObject(parent)
    , m_reply(reply)
    , m_index(index)
{
    if (m_reply)
    {
        connect(m_reply, &QNetworkReply::downloadProgress, this, &StreamReply::downloadProgress);
        connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
        connect(m_reply, &QNetworkReply::finished, this, &StreamReply::finished);
        connect(m_reply, &QNetworkReply::sslErrors, this, &StreamReply::sslErrors);
        connect(m_reply, &QNetworkReply::uploadProgress, this, &StreamReply::uploadProgress);
        connect(m_reply, &QNetworkReply::readyRead, this, &StreamReply::readyRead);
    }
}

StreamReply::~StreamReply()
{
    if (m_reply)
    {
        m_reply->disconnect(this);
        m_reply->deleteLater();
        m_reply = NULL;
    }
}

void StreamReply::stop()
{
    if (m_reply)
    {
        m_reply->abort();
        emit cancel();
    }
}

void StreamReply::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);
}

void StreamReply::error(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);
    if (m_reply)
    {
        QString e = m_reply->errorString();
#if !defined(QT_NO_DEBUG)
        qDebug() << __FUNCTION__ << e;
#endif
        emit errorMessage(code, e);
    }
}

void StreamReply::finished()
{
#if !defined(QT_NO_DEBUG)
    qDebug() << this << " finished: " << QString(m_content) << "\n";
#endif

    emit done();
}

void StreamReply::sslErrors(const QList<QSslError> &errors)
{
#if !defined(QT_NO_DEBUG)
    Q_FOREACH(const QSslError &e, errors)
    {
        qDebug() << "ssl error:" << e.errorString();
    }
#endif
}

void StreamReply::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    Q_UNUSED(bytesSent);
    Q_UNUSED(bytesTotal);
}

void StreamReply::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        reply->readAll();
    }
}

int StreamReply::index() const
{
    return m_index;
}
