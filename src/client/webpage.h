#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>

class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    WebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    bool certificateError(const QWebEngineCertificateError &error) override;
private slots:
    void onAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth);
    void onProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost);
};

#endif // WEBPAGE_H
