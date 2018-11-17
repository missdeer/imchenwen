#ifndef URLREQUESTINTERCEPTOR_H
#define URLREQUESTINTERCEPTOR_H

#include <QObject>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
#include "config.h"

class UrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit UrlRequestInterceptor(QObject *parent = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo &request) override;
signals:
    void maybeMediaUrl(const QString&);

private:
    Tuple2List m_vipVideos;
};

#endif // URLREQUESTINTERCEPTOR_H
