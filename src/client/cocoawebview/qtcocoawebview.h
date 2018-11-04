#ifndef QTCOCOAWEBVIEW_H
#define QTCOCOAWEBVIEW_H

#include <QMacCocoaViewContainer>

class QtCocoaWebView : public QMacCocoaViewContainer
{
    Q_OBJECT
public:
    explicit QtCocoaWebView(QWidget *parent = nullptr);
    explicit QtCocoaWebView(QString loadUrl, QWidget *parent = nullptr);

    void loadFinish(const QString &strUrl, const QString &strHtml);
    void loadError();
    void urlChanged(const QString &strUrl);
    void loadRequest(const QString &loadUrl);

signals:
    void signalLoadFinish(const QString &strUrl, const QString &strHtml);
    void signalLoadError();
    void signalUrlChanged(const QString &strUrl);
};

#endif // WEBVIEWINQT_H
