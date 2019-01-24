#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include <QObject>
#include "linkresolverprocess.h"

class StorageService : public QObject
{
    Q_OBJECT
public:
    explicit StorageService(QObject *parent = nullptr);
    void submit(StreamInfoPtr video, StreamInfoPtr audio, const QString &subtitle, const QString &title, const QString &referrer);
    void submit(const QString &videoUrl, const QString &title);

signals:

private slots:
    void onSubmitted();

private:
    QString baseName(const QString &base);
    QUuid doSubmit(const QString &baseUrl, const QString &targetLink, const QString &saveAs, const QString &referrer);
};

#endif // STORAGESERVICE_H
