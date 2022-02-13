#ifndef LINKRESOLVERPROCESS_H
#define LINKRESOLVERPROCESS_H

#include <QProcess>
#include <QTime>
#include <QSharedPointer>
#include <QJsonValue>
#include "mediainfo.h"

class LinkResolverProcess : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolverProcess(QObject *parent = nullptr);
    virtual ~LinkResolverProcess();
    virtual void setProgram(const QString& program);
    virtual void start(const QString &url);
    virtual void parseNode(const QJsonObject &, MediaInfoPtr) = 0;
    virtual void init()                                       = 0;
    virtual void resolved(MediaInfoPtr)                       = 0;
signals:
    void done(const QByteArray&);
public slots:
    void stop();
private slots:
    void onReadStandardOutput();
    void onFinished(int, QProcess::ExitStatus);
protected:
    QProcess m_process;
    QByteArray m_data;
    QStringList m_args;
    int m_timeout;
    bool needProxy(const QString& url);
};

#endif // LINKRESOLVERPROCESS_H
