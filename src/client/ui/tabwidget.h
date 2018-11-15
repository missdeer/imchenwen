#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QWebEnginePage>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class WebView;

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);
    ~TabWidget();

    WebView *currentWebView() const;
    WebView *navigateInNewTab(const QUrl &url, bool makeCurrent = true);
signals:
    // current tab/page signals
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void iconChanged(const QIcon &icon);
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

public slots:
    // current tab/page slots
    void onSetUrl(const QUrl &url);
    void onTriggerWebPageAction(QWebEnginePage::WebAction action);

    WebView *onCreateTab(bool makeCurrent = true);
    void onCloseTab(int index);
    void onNextTab();
    void onPreviousTab();

private slots:
    void onCurrentChanged(int index);
    void onContextMenuRequested(const QPoint &pos);
    void onCloneTab(int index);
    void onCloseOtherTabs(int index);
    void onReloadAllTabs();
    void onReloadTab(int index);

private:
    WebView *webView(int index) const;
    void setupView(WebView *webView);
};

#endif // TABWIDGET_H
