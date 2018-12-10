#ifndef URLREQUESTINTERCEPTOR_H
#define URLREQUESTINTERCEPTOR_H

#include <QObject>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>

class UrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit UrlRequestInterceptor(bool mpv, QObject *parent = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo &request) override;
signals:
    void maybeMediaUrl(const QString&);

private:
    bool m_mpv;
    void outputToStdout(const QString& output);
    void playByMPV(const QString& output);
};

#endif // URLREQUESTINTERCEPTOR_H
