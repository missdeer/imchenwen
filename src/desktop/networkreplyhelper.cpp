#include <QtCore>
#include "networkreplyhelper.h"

NetworkReplyHelper::NetworkReplyHelper(QNetworkReply *reply, QObject *parent)
    : QObject(parent)
    , m_reply(reply)
{
    if (m_reply)
    {
        connect(m_reply, &QNetworkReply::downloadProgress, this, &NetworkReplyHelper::downloadProgress);
        connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
        connect(m_reply, &QNetworkReply::finished, this, &NetworkReplyHelper::finished);
        connect(m_reply, &QNetworkReply::sslErrors, this, &NetworkReplyHelper::sslErrors);
        connect(m_reply, &QNetworkReply::uploadProgress, this, &NetworkReplyHelper::uploadProgress);
        connect(m_reply, &QNetworkReply::readyRead, this, &NetworkReplyHelper::readyRead);
    }
}

NetworkReplyHelper::~NetworkReplyHelper()
{
    if (m_reply)
    {
        m_reply->disconnect(this);
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void NetworkReplyHelper::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);
}

void NetworkReplyHelper::error(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);
    if (m_reply)
    {
        QString e = m_reply->errorString();
        qDebug() << __FUNCTION__ << e;
        emit errorMessage(code, e);
    }
}

void NetworkReplyHelper::finished()
{
#if !defined(QT_NO_DEBUG)
    qDebug() << this << " finished: " << QString(m_content) << "\n";
#endif

    emit done();
}

void NetworkReplyHelper::sslErrors(const QList<QSslError> &errors)
{
    Q_FOREACH(const QSslError &e, errors)
    {
        qDebug() << "ssl error:" << e.errorString();
    }
}

void NetworkReplyHelper::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    Q_UNUSED(bytesSent);
    Q_UNUSED(bytesTotal);
}

void NetworkReplyHelper::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        m_content.append( reply->readAll());
    }
}

