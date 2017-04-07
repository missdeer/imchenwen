#include "streamreply.h"
#include <QStandardPaths>
#include <QFile>
#include <QEventLoop>
#include <QTimer>

StreamReply::StreamReply(int index, QNetworkReply *reply, QObject *parent)
    : QObject(parent)
    , m_reply(reply)
    , m_index(index)
    , m_finished(false)
    , m_localReadyRead(false)
{
    Q_ASSERT(m_reply);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &StreamReply::downloadProgress);
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
    connect(m_reply, &QNetworkReply::finished, this, &StreamReply::finished);
    connect(m_reply, &QNetworkReply::sslErrors, this, &StreamReply::sslErrors);
    connect(m_reply, &QNetworkReply::uploadProgress, this, &StreamReply::uploadProgress);
    connect(m_reply, &QNetworkReply::readyRead, this, &StreamReply::remoteReadyRead);

    m_cachePath = QString("%1/imchenwencache-%2").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).arg(index);
    m_in = new QFile(m_cachePath);
    m_in->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered);
}

StreamReply::~StreamReply()
{
    if (m_reply)
    {
        m_reply->disconnect(this);
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_in)
    {
        m_in->close();
        delete m_in;
        m_in = nullptr;
    }
    if (m_out)
    {
        m_out->close();
        delete m_out;
        m_out = nullptr;
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

QByteArray StreamReply::read()
{
    qDebug() << __FUNCTION__;
    if (!m_out)
    {
        if (!m_localReadyRead)
        {
            QTimer timer;
            timer.setSingleShot(true);
            QEventLoop loop;
            connect(this,  SIGNAL(localReadyRead()), &loop, SLOT(quit()) );
            connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            timer.start(10*1000);
            loop.exec();
        }
        m_out = new QFile(m_cachePath);
        m_out->open(QIODevice::ReadOnly );
    }
    return m_out->read(64 * 1024);
}

bool StreamReply::atEnd()
{
    qDebug() << __FUNCTION__;
    if (m_finished && m_out)
        return m_out->atEnd();
    return false;
}

const QList<QNetworkReply::RawHeaderPair> &StreamReply::rawHeaderPairs() const
{
    return m_reply->rawHeaderPairs();
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
        qDebug() << __FUNCTION__ << e;
        emit errorMessage(code, e);
    }
}

void StreamReply::finished()
{
    qDebug() << this << " finished";
    m_finished = true;
    emit done();
}

void StreamReply::sslErrors(const QList<QSslError> &errors)
{
    Q_FOREACH(const QSslError &e, errors)
    {
        qDebug() << "ssl error:" << e.errorString();
    }
}

void StreamReply::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    Q_UNUSED(bytesSent);
    Q_UNUSED(bytesTotal);
}

void StreamReply::remoteReadyRead()
{
    qDebug() << __FUNCTION__;
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    m_statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (m_statusCode >= 200 && m_statusCode < 300) {
        m_in->write(reply->readAll());
        if (!m_localReadyRead)
        {
            m_localReadyRead = true;
            emit localReadyRead();
        }
    }
}

int StreamReply::statusCode() const
{
    return m_statusCode;
}

int StreamReply::index() const
{
    return m_index;
}
