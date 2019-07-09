#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

#if defined(Q_OS_MAC)
#include <qtcocoawebview.h>

using ImWebView = QtCocoaWebView;
#else
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include "webpage.h"
#include "webview.h"

using ImWebView = WebView;
#endif

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE


class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);
    TabWidget(const TabWidget &) = delete;
    TabWidget(TabWidget &&) = delete;
    TabWidget& operator=(const TabWidget &) = delete;
    TabWidget& operator=(TabWidget &&) = delete;
    ~TabWidget() override = default;

    ImWebView *currentWebView() const;
    ImWebView *navigateInNewTab(const QUrl &url, bool makeCurrent = true);
    void closeAllTabs();
signals:
    // current tab/page signals
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void iconChanged(const QIcon &icon);
    
#if defined(Q_OS_MAC)
#else
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
#endif

public slots:
    // current tab/page slots
    void onSetUrl(const QUrl &url);
    
#if defined (Q_OS_MAC)
#else
    void onTriggerWebPageAction(QWebEnginePage::WebAction action);
#endif

    ImWebView *onCreateTab(bool makeCurrent = true);
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
    ImWebView *webView(int index) const;
    void setupView(ImWebView *webView);
};

#endif // TABWIDGET_H
