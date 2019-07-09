#ifndef QTCOCOAWEBVIEW_H
#define QTCOCOAWEBVIEW_H

#include <QMacCocoaViewContainer>
#include <QUrl>
#include <QIcon>

class QtCocoaWebView : public QMacCocoaViewContainer
{
    Q_OBJECT
public:
    explicit QtCocoaWebView(QWidget *parent = nullptr);
    explicit QtCocoaWebView(const QString& loadUrl, QWidget *parent = nullptr);

    void loadRequest(const QString &loadUrl);
    bool canGoBack();
    bool canGoForward();
    void back();
    void forward();
    void reload();

    void onLoadFinish(const QString &strUrl, const QString &strHtml);
    void onLoadError();
    void onUrlChanged(const QString &strUrl);
    void onTitleChanged(const QString &strTitle);
    void onIconChanged(const QIcon& icon);
    void onNewWindowsRequest(const QString &strUrl);
    void onStatusTextChanged(const QString &text);
    
    QUrl url() const;
    void setUrl(const QUrl& u);
    QString title() const;
    int loadProgress();
    
    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);
    
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    QIcon icon();
signals:
    void loadFinish(const QString &strUrl, const QString &strHtml);
    void loadError();
    void urlChanged(const QString &strUrl);
    void titleChanged(const QString &strTitle);
    void iconChanged(const QIcon& icon);
    void newWindowsRequest(const QString &strUrl);
    void statusTextChanged(const QString &text);
    
private:
    QUrl m_url;
    QString m_title;
    QIcon m_icon;
};

#endif // WEBVIEWINQT_H
