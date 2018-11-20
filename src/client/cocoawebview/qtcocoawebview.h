#ifndef QTCOCOAWEBVIEW_H
#define QTCOCOAWEBVIEW_H

#include <QMacCocoaViewContainer>

class QtCocoaWebView : public QMacCocoaViewContainer
{
    Q_OBJECT
public:
    explicit QtCocoaWebView(QWidget *parent = nullptr);
    explicit QtCocoaWebView(QString loadUrl, QWidget *parent = nullptr);

    void loadRequest(const QString &loadUrl);
    bool canGoBack();
    bool canGoForward();
    void goBack();
    void goForward();

    void onLoadFinish(const QString &strUrl, const QString &strHtml);
    void onLoadError();
    void onUrlChanged(const QString &strUrl);
    void onTitleChanged(const QString &strTitle);
    void onIconChanged(const QIcon& icon);
    void onNewWindowsRequest(const QString &strUrl);
    void onStatusTextChanged(const QString &text);
signals:
    void loadFinish(const QString &strUrl, const QString &strHtml);
    void loadError();
    void urlChanged(const QString &strUrl);
    void titleChanged(const QString &strTitle);
    void iconChanged(const QIcon& icon);
    void newWindowsRequest(const QString &strUrl);
    void statusTextChanged(const QString &text);
};

#endif // WEBVIEWINQT_H
